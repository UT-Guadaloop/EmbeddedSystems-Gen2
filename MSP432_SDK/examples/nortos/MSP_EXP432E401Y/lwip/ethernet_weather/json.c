//*****************************************************************************
//
// json.c - File to handle JSON formatted data from websites.
//
// Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ustdlib.h"
#include "lwiplib.h"
#include "eth_client.h"
#include "json.h"
#include "images.h"
//##### INTERNAL BEGIN #####
#include "debug.h"
//##### INTERNAL END #####

//****************************************************************************
//
// The locally defined lwIP buffer parsing pointer.
//
//****************************************************************************
typedef struct
{
    //
    // A pbuf pointer from lwIP.
    //
    struct pbuf *psBuf;

    //
    // The current index into the pbuf pointer.
    //
    uint32_t ui32Idx;
}
tBufPtr;

//****************************************************************************
//
// Initialize a buffer parsing pointer.
//
//****************************************************************************
static void
BufPtrInit(tBufPtr *psBufPtr, struct pbuf *psBuf)
{
    psBufPtr->psBuf = psBuf;
    psBufPtr->ui32Idx = 0;
}

//****************************************************************************
//
// Get a single byte from a parsing pointer.
//
//****************************************************************************
static uint8_t
BufData8Get(tBufPtr *psBufPtr)
{
    return(((uint8_t *)psBufPtr->psBuf->payload)[psBufPtr->ui32Idx]);
}

//*****************************************************************************
//
// Increment a parsing pointer by a given value.
//
//*****************************************************************************
uint32_t
BufPtrInc(tBufPtr *psBufPtr, uint32_t ui32Inc)
{
    psBufPtr->ui32Idx += ui32Inc;

    if(psBufPtr->ui32Idx >= psBufPtr->psBuf->len)
    {
        psBufPtr->ui32Idx = 0;

        psBufPtr->psBuf = psBufPtr->psBuf->next;

        if(psBufPtr->psBuf == 0)
        {
            ui32Inc = 0;
        }
    }
    return(ui32Inc);
}

//*****************************************************************************
//
// Return the image to use for the given icon returned from the weather update.
//
//*****************************************************************************
void
GetImage(char *pcIcon, const uint8_t **ppui8Image, const char **ppcDescription)
{
    static const struct
    {
        char pcId[2];
        const uint8_t *pui8Image;
        const char *pcDescription;
    }
    pcIconTable[9] =
    {
        //
        // Clear Sky.
        //
        { "01", g_pui8SunImage, "Clear Sky"},

        //
        // Light Clouds.
        //
        { "02", g_pui8CloudyImage, "Light Clouds"},

        //
        // Scattered Clouds.
        //
        { "03", g_pui8CloudyImage, "Scattered Clouds"},

        //
        // Broken Clouds.
        //
        { "04", g_pui8CloudyImage, "Broken Clouds"},

        //
        // Rain showers.
        //
        { "09", g_pui8RainImage, "Light Rain"},

        //
        // Rain.
        //
        { "10", g_pui8RainImage, "Rain"},

        //
        // Thunderstorms.
        //
        { "11", g_pui8ThuderStormImage, "Thunderstorms"},

        //
        // Snow.
        //
        { "13", g_pui8SnowImage, "Snow"},

        //
        // Mist/Fog.
        //
        { "50", g_pui8FogImage, "Mist/Fog"},
    };
    int32_t i32Idx;

    for(i32Idx = 0; i32Idx < 9; i32Idx++)
    {
        if((pcIcon[0] == pcIconTable[i32Idx].pcId[0]) &&
           (pcIcon[1] == pcIconTable[i32Idx].pcId[1]))
        {
            *ppui8Image = pcIconTable[i32Idx].pui8Image;
            *ppcDescription = pcIconTable[i32Idx].pcDescription;
            break;
        }
    }
}

//*****************************************************************************
//
// Compare a string at the current location of a parsing pointer.
//
//*****************************************************************************
uint32_t
CompareString(tBufPtr *psBufPtr, const char *pcField, uint32_t ui32Size)
{
    uint32_t ui32Idx;

    for(ui32Idx = 0; ui32Idx < ui32Size; ui32Idx++)
    {
//##### INTERNAL BEGIN #####
//        DebugPrintf("%c == %c\n", BufData8Get(psBufPtr), pcField[ui32Idx]);
//##### INTERNAL END #####
        if(BufData8Get(psBufPtr) == pcField[ui32Idx])
        {
            if(BufPtrInc(psBufPtr, 1) != 1)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    if(ui32Idx == ui32Size)
    {
        return(0);
    }

    return(1);
}

//*****************************************************************************
//
// This function searches for a field from the JSON data.  The field should be
// quoted in the data and immediately follow a '{' character.  This can be used
// to also search for sub data in another field if the pointer to the data for
// the parent field.
//
//*****************************************************************************
static uint32_t
GetField(char *pcField, tBufPtr *psBufPtr)
{
    uint32_t ui32Curly, ui32Quote;
    int32_t i32NewItem;

    ui32Curly = 0;
    i32NewItem = 0;
    ui32Quote = 0;

//##### INTERNAL BEGIN #####
//    DebugPrintf("GetField %s\n", pcField);
//##### INTERNAL END #####
    while(1)
    {
        if(BufData8Get(psBufPtr) == '{')
        {
            ui32Curly++;
        }
        else if(BufData8Get(psBufPtr) == '}')
        {
            ui32Curly--;

            if(ui32Curly == 1)
            {
                ui32Quote = 0;
            }
        }
        else if(BufData8Get(psBufPtr) == ',')
        {
            if((ui32Curly == 1) && ((ui32Quote & 1) == 0))
            {
                ui32Quote = 0;
            }
        }
        else if(BufData8Get(psBufPtr) == '"')
        {
            if((ui32Curly == 1) && (ui32Quote == 0))
            {
                i32NewItem = 1;
            }
            ui32Quote++;
        }

        if(BufPtrInc(psBufPtr, 1) != 1)
        {
            break;
        }

        if(i32NewItem)
        {
            i32NewItem = 0;
//##### INTERNAL BEGIN #####
//            DebugPrintf("%c\n", BufData8Get(psBufPtr));
//##### INTERNAL END #####
            if(CompareString(psBufPtr, pcField, ustrlen(pcField)) == 0)
            {
                while(1)
                {
                    if(BufData8Get(psBufPtr) == ':')
                    {
                        if(BufPtrInc(psBufPtr, 1) != 1)
                        {
                            break;
                        }
                        return(1);
                    }

                    if(BufPtrInc(psBufPtr, 1) != 1)
                    {
                        break;
                    }
                }
                return(0);
            }
        }
    }
    return(0);
}

//*****************************************************************************
//
// This function searches for a value in a JSON item as an integer.  These are
// non-quoted values so if the number is a quoted integer this function returns
// -1.  If the value is found then it is returned by this function.
//
//*****************************************************************************
int32_t
GetFieldValueInt(tBufPtr *psBufPtr)
{
    int32_t i32Idx;
    char pcTemp[20];
    const char *pEnd;

    i32Idx = 0;

    while(1)
    {
        //
        // Find the end of this item.
        //
        if((BufData8Get(psBufPtr) == '}') || (BufData8Get(psBufPtr) == ','))
        {
            //
            // null terminate the string and get the value from the string in
            // decimal.
            //
            pcTemp[i32Idx] = 0;
            return(ustrtoul(pcTemp, &pEnd, 10));
        }

        //
        // Should not hit a " char or something went wrong.
        //
        if(BufData8Get(psBufPtr) == '"')
        {
            break;
        }

        //
        // Copy the string to the local variable.
        //
        pcTemp[i32Idx] = BufData8Get(psBufPtr);

        if((pcTemp[i32Idx] == '\r') || (pcTemp[i32Idx] == '\n'))
        {
            pcTemp[i32Idx] = 0;
        }
        else
        {
            i32Idx++;
        }

        if(BufPtrInc(psBufPtr, 1) != 1)
        {
            break;
        }

        //
        // Number was too large to represent.
        //
        if(i32Idx >= sizeof(pcTemp))
        {
            break;
        }
    }
    return(INVALID_INT);
}

//*****************************************************************************
//
// This function searches for a value in a JSON item as quoted value.  These
// values are quoted values so if the number is not a quoted value this
// function returns -1.  If the quoted value is found then it is returned in
// the pcDataDest array.
//
//*****************************************************************************
int32_t
GetFieldValueString(tBufPtr *psBufPtr, char *pcDataDest, uint32_t ui32SizeDest)
{
    int32_t i32OutIdx;

    //
    // The value should always start with a " char or something went wrong.
    //
    if(BufData8Get(psBufPtr) != '"')
    {
        return(-1);
    }

    //
    // Skip the initial " char.
    //
    if(BufPtrInc(psBufPtr, 1) != 1)
    {
        return(-1);
    }

    for(i32OutIdx = 0; i32OutIdx < ui32SizeDest;)
    {
        //
        // Either a '}', ',', or '"' ends an item.
        //
        if((BufData8Get(psBufPtr) == '}') ||
           (BufData8Get(psBufPtr) == ',') ||
           (BufData8Get(psBufPtr) == '"'))
        {
            //
            // Null terminate the string and return.
            //
            pcDataDest[i32OutIdx] = 0;
            return(i32OutIdx);
        }

        //
        // Continue coping chars into the destination buffer.
        //
        pcDataDest[i32OutIdx] = BufData8Get(psBufPtr);

        //
        // These can occur in the response string and need to be ignored.
        //
        if((pcDataDest[i32OutIdx] == '\r') || (pcDataDest[i32OutIdx] == '\n'))
        {
            pcDataDest[i32OutIdx] = 0;
        }
        else
        {
            i32OutIdx++;
        }

        if(BufPtrInc(psBufPtr, 1) != 1)
        {
            break;
        }
    }

    //
    // Make sure to null terminate inside the current string.
    //
    if(i32OutIdx == ui32SizeDest)
    {
        pcDataDest[i32OutIdx-1] = 0;
        return(i32OutIdx - 1);
    }
    return(-1);
}

//*****************************************************************************
//
// Fill out the psWeatherReport structure from data returned from the JSON
// query.
//
//*****************************************************************************
int32_t
JSONParseForecast(uint32_t ui32Index, tWeatherReport *psWeatherReport,
                  struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufList, sBufTemp;
    char pcCode[4];
    int32_t i32Items,i32Code;

//##### INTERNAL BEGIN #####
    DebugPrintf("\nJSONParseForecast\n");
//##### INTERNAL END #####

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if(GetField("cod", &sBufPtr) != 0)
    {
        //
        // Check for a 404 not found error.
        //
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if(i32Code != INVALID_INT)
        {
            if(i32Code == 404)
            {
                return(-1);
            }
        }
        else if(GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 not found error.
            //
            if(ustrncmp(pcCode, "404", 3) == 0)
            {
                return(-1);
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Initialize the number of items found.
    //
    i32Items = 0;

    if(GetField("list", &sBufPtr) != 0)
    {
        //
        // Save the list pointer.
        //
        sBufList = sBufPtr;

        //
        // Get the humidity.
        //
        if(GetField("humidity", &sBufPtr) != 0)
        {
            psWeatherReport->i32Humidity = GetFieldValueInt(&sBufPtr);
            i32Items++;
        }
        else
        {
            psWeatherReport->i32Humidity = INVALID_INT;
        }

        //
        // Reset the pointer to the start of the list values.
        //
        sBufPtr = sBufList;

        //
        // Get the average daily pressure.
        //
        if(GetField("pressure", &sBufPtr) != 0)
        {
            psWeatherReport->i32Pressure = GetFieldValueInt(&sBufPtr);
            i32Items++;
        }
        else
        {
            psWeatherReport->i32Pressure = INVALID_INT;
        }

        //
        // Reset the pointer to the start of the list values.
        //
        sBufPtr = sBufList;

        //
        // Get the average daily temperature.
        //
        if(GetField("temp", &sBufPtr) != 0)
        {
            if(GetField("day", &sBufPtr) != 0)
            {
                psWeatherReport->i32Temp = GetFieldValueInt(&sBufPtr);
                i32Items++;
            }
            else
            {
                psWeatherReport->i32Temp = INVALID_INT;
            }
        }

        //
        // Reset the pointer to the start of the list values.
        //
        sBufPtr = sBufList;

        //
        // Get the low temperature.
        //
        if(GetField("temp", &sBufPtr) != 0)
        {
            if(GetField("min", &sBufPtr) != 0)
            {
                psWeatherReport->i32TempLow = GetFieldValueInt(&sBufPtr);
                i32Items++;
            }
            else
            {
                psWeatherReport->i32TempLow = INVALID_INT;
            }
        }

        //
        // Reset the pointer to the start of the list values.
        //
        sBufPtr = sBufList;

        //
        // Get the high temperature.
        //
        if(GetField("temp", &sBufPtr) != 0)
        {
            if(GetField("max", &sBufPtr) != 0)
            {
                psWeatherReport->i32TempHigh = GetFieldValueInt(&sBufPtr);
                i32Items++;
            }
            else
            {
                psWeatherReport->i32TempHigh = INVALID_INT;
            }
        }
        //
        // Reset the pointer to the start of the list values.
        //
        sBufPtr = sBufList;

        if(GetField("dt", &sBufPtr) != 0)
        {
            psWeatherReport->ui32Time = GetFieldValueInt(&sBufPtr);
            i32Items++;
        }
        else
        {
            psWeatherReport->ui32Time = 0;
        }
    }

    return(i32Items);
}

//*****************************************************************************
//
// Fill out the psWeatherReport structure from data returned from the JSON
// query.
//
//*****************************************************************************
int32_t
JSONParseCurrent(uint32_t ui32Index, tWeatherReport *psWeatherReport,
                 struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufMain, sBufTemp;
    char pcIcon[3];
    char pcCode[4];
    int32_t i32Items, i32Code;
//##### INTERNAL BEGIN #####
    DebugPrintf("\nJSONParseCurrent\n");
//##### INTERNAL END #####

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if(GetField("cod", &sBufPtr) != 0)
    {
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if(i32Code != INVALID_INT)
        {
            if(i32Code == 404)
            {
                return(-1);
            }
        }
        else if(GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 not found error.
            //
            if(ustrncmp(pcCode, "404", 3) == 0)
            {
                return(-1);
            }
        }
    }

    //
    // Initialize the number of items that have been found.
    //
    i32Items = 0;

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Find the basic weather description(weather.description).
    //
    if(GetField("weather", &sBufPtr) != 0)
    {
        if(GetField("icon", &sBufPtr) != 0)
        {
            if(GetFieldValueString(&sBufPtr, pcIcon, sizeof(pcIcon)) > 0)
            {
                //
                // Save the image pointer.
                //
                GetImage(pcIcon, &psWeatherReport->pui8Image,
                         &psWeatherReport->pcDescription);

                i32Items++;
            }
            else
            {
                //
                // No image was found.
                //
                psWeatherReport->pui8Image = 0;
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Find the humidity value(main.humidity).
    //
    if(GetField("sys", &sBufPtr) != 0)
    {
        //
        // Save the pointer to the data.
        //
        sBufMain = sBufPtr;

        //
        // Get the sunrise time.
        //
        if(GetField("sunrise", &sBufPtr) != 0)
        {
            psWeatherReport->ui32SunRise = GetFieldValueInt(&sBufPtr);

            i32Items++;
        }
        else
        {
            psWeatherReport->ui32SunRise = 0;
        }

        //
        // Restore the pointer to the data.
        //
        sBufPtr = sBufMain;

        //
        // Get the sunset time.
        //
        if(GetField("sunset", &sBufPtr) != 0)
        {
            psWeatherReport->ui32SunSet = GetFieldValueInt(&sBufPtr);

            i32Items++;
        }
        else
        {
            psWeatherReport->ui32SunSet = 0;
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Get the last update time.
    //
    if(GetField("dt", &sBufPtr) != 0)
    {
        psWeatherReport->ui32Time = GetFieldValueInt(&sBufPtr);

        i32Items++;
    }
    else
    {
        psWeatherReport->ui32Time = 0;
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Find the humidity value(main.humidity).
    //
    if(GetField("main", &sBufPtr) != 0)
    {
        //
        // Save the pointer to the main data.
        //
        sBufMain = sBufPtr;

        if(GetField("humidity", &sBufPtr) != 0)
        {
            psWeatherReport->i32Humidity = GetFieldValueInt(&sBufPtr);
            i32Items++;
        }
        else
        {
            psWeatherReport->i32Humidity = INVALID_INT;
        }

        //
        // Reset the buffer pointer to main.
        //
        sBufPtr = sBufMain;

        //
        // Find the current temperature value.
        //
        if(GetField("temp", &sBufPtr) != 0)
        {
            psWeatherReport->i32Temp = GetFieldValueInt(&sBufPtr);
            i32Items++;
        }
        else
        {
            psWeatherReport->i32Temp = INVALID_INT;
        }

        //
        // Reset the buffer pointer to main.
        //
        sBufPtr = sBufMain;

        //
        // Find the current atmospheric pressure.
        //
        if(GetField("pressure", &sBufPtr) != 0)
        {
            psWeatherReport->i32Pressure = GetFieldValueInt(&sBufPtr);
            i32Items++;
        }
        else
        {
            psWeatherReport->i32Pressure = INVALID_INT;
        }
    }
    return(i32Items);
}
//##### INTERNAL BEGIN #####

#ifdef API_TEST
char pcLondonTest[]= "{\"coord\":{\"lon\":-0.12574,\"lat\":51.50853},\"sys\":"
                     "{\"country\":\"GB\",\"sunrise\":1371094963,\"sunset\":"
                     "1371154721},\"weather\":[{\"id\":500,\"main\":\"Rain\","
                     "\"description\":\"light rain\",\"testvalue\":\"ABCDEFGH"
                     "IJKLMNOPQRSTUVWXYZ\",\"icon\":\"10d\"}],\"base\":\""
                     "global stations\",\"main\":{\"temp\":289.33,\"humidity\":"
                     "63,\"pressure\":1011,\"temp_min\":288.15,\"temp_max\":"
                     "290.15},\"wind\":{\"speed\":3.6,\"gust\":6.17,\"deg\":"
                     "284},\"rain\":{\"3h\":1},\"clouds\":{\"all\":64},\"dt\":"
                     "1371145595,\"id\":2643743,\"name\":\"London\",\"cod\":"
                     "200}";

int32_t
JSONDataTest(void)
{
    int32_t i32Idx, i32String, i32Ret;
    uint32_t ui32Size;
    char pcTestString[80];

    i32Ret = 0;

    ui32Size = sizeof(pcLondonTest);

    //
    // Get the name of the city.
    //
    i32Idx = GetField("xyz", pcLondonTest, ui32Size);

    if(i32Idx != 0)
    {
        printf("Error: GetField() invalid query returned %d\n", i32Idx);
        i32Ret--;
    }

    //
    // Get the name of the city.
    //
    i32Idx = GetField("name", pcLondonTest, ui32Size);
    i32String = GetFieldValueString(&pcLondonTest[i32Idx], ui32Size - i32Idx,
                                    pcTestString, 80);
    if((i32String <= 0) || (strncmp(pcTestString, "London", 80) != 0))
    {
        printf("Error: GetFieldValueString() name query returned %d\n",
                i32String);
        i32Ret--;
    }

    //
    // Get the value of the city as an integer, this should fail.
    //
    i32String = GetFieldValueInt(&pcLondonTest[i32Idx], ui32Size - i32Idx);
    if(i32String != -1)
    {
        printf("Error: GetFieldValueInt() name as int query returned %d\n",
                i32String);
        i32Ret--;
    }

    i32Idx = GetField("dt", pcLondonTest, ui32Size);
    i32String = GetFieldValueInt(&pcLondonTest[i32Idx], ui32Size - i32Idx);

    if(i32String <= 0)
    {
        printf("Error: GetFieldValueInt() dt query returned %d\n",
                i32String);
        i32Ret--;
    }
    i32String = GetFieldValueString(&pcLondonTest[i32Idx], ui32Size - i32Idx,
                                    pcTestString, 80);

    if(i32String > 0)
    {
        printf("Error: GetFieldValueString() dt query string returned %d\n",
                i32String);
        i32Ret--;
    }

    //
    // Get a valid string from a sub entry.
    //
    i32Idx = GetField("weather", pcLondonTest, ui32Size);
    i32Idx += GetField("description", &pcLondonTest[i32Idx], ui32Size - i32Idx);
    i32String = GetFieldValueString(&pcLondonTest[i32Idx], ui32Size - i32Idx,
                                    pcTestString, 80);
    if((i32String <= 0) || (strncmp(pcTestString, "light rain", 80) != 0))
    {
        printf("Error: GetFieldValueString() weather.description query string"
               "returned %d\n", i32String);
        i32Ret--;
    }

    //
    // Invalid request for an integer from a string value.
    //
    i32String = GetFieldValueInt(&pcLondonTest[i32Idx], ui32Size - i32Idx);
    if(i32String >= 0)
    {
        printf("Error: GetFieldValueInt() weather.description query int "
               "returned %d\n", i32String);
        i32Ret--;
    }

    //
    // Valid request for an integer from a sub entry.
    //
    i32Idx = GetField("main", pcLondonTest, ui32Size);
    i32Idx += GetField("temp", &pcLondonTest[i32Idx], ui32Size - i32Idx);
    i32String = GetFieldValueInt(&pcLondonTest[i32Idx], ui32Size - i32Idx);
    if((i32String <= 0) || (i32String != 289))
    {
        printf("Error: GetFieldValueInt() main.temp query returned %d\n",
                i32String);
        i32Ret--;
    }

    //
    // Invalid request for an string from an integer sub entry.
    //
    i32String = GetFieldValueString(&pcLondonTest[i32Idx], ui32Size - i32Idx,
                                    pcTestString, 80);
    if(i32String >= 0)
    {
        printf("Error: GetFieldValueString() main.temp query returned %d\n",
                i32String);
        i32Ret--;
    }

    //
    // Get a valid string but with not enough space to hold the full string.
    //
    i32Idx = GetField("weather", pcLondonTest, ui32Size);
    i32Idx += GetField("testvalue", &pcLondonTest[i32Idx], ui32Size - i32Idx);
    i32String = GetFieldValueString(&pcLondonTest[i32Idx], ui32Size - i32Idx,
                                    pcTestString, 10);
    if((i32String <= 0) || (strncmp(pcTestString, "ABCDEFGHI", 80) != 0))
    {
        printf("Error: GetFieldValueString() weather.description query string"
               "returned %d\n", i32String);
        i32Ret--;
    }

    return(i32Ret);

}
#endif
//##### INTERNAL END #####
