/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
 /*
 *  ======== cloud_task.c ========
 */

/* Standard header files */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>

/* Driver header files */
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpclient.h>
#include <ti/net/slnetutils.h>

/* Example/Board Header files */
#include "ti_drivers_config.h"
#include "board_funcs.h"
#include "cloud_task.h"
#include "command_task.h"

/* Stack size of the Cloud task.  This task needs a big stack as WolfSSL
 * function calls require bigger stack. */
#define STACK_CLOUD_TASK        20000

/* Sys BIOS Frequency is again define here as it is needed to calculate time
 * since start. */
#define BIOS_TICK_RATE          1000

/* Defines used by POST and GET requests. */
#define PROVISION_URI           "/provision/activate"
#define EXOSITE_URI             "/onep:v1/stack/alias"
#define EXOSITE_CONTENT_TYPE    "application/x-www-form-urlencoded; "         \
                                "charset=utf-8"
#define EXOSITE_TYPE            "X-Exosite-CIK"

/* Global resource to hold Exosite IP address. */
#define EXOSITE_URL_LEN         50
char g_pcExositeURL[EXOSITE_URL_LEN];

/* Global resource to store Exosite CIK. */
static char g_pcExositeCIK[EXOSITE_CIK_LENGTH + 1] = {0};

/* Global resource to store Exosite Product ID. */
static char g_pcExositePID[EXOSITE_PID_LENGTH + 1] = {0};

/* Global resource to store MAC Address. */
char g_pcMACAddress[MAC_ADDRESS_LENGTH + 1];

/* Global resource to store IP Address. */
uint32_t g_ui32IPAddr;

/* An array to hold the data to be displayed on UART console. */
tMailboxMsg g_sDebug;

/* The alias sent to Exosite server with POST request. */
const char g_ppcPOSTAlias[6][15] =
{
    "usrsw1",
    "usrsw2",
    "temp",
    "ontime",
    "led"
};

/* The alias values requested from Exosite server with GET request. */
const char g_ppcGETAlias[ALIAS_PROCESSING][15] =
{
    "led"
};

/* The alias sent to Exosite server while provisioning for a CIK. */
const char g_ppcProvAlias[3][10] =
{
    "vendor",
    "model",
    "sn"
};

/* Global resources to hold led state and read/write mode. */
uint32_t g_ui32LEDD1 = 0;
uint32_t g_ui32LastLEDD1 = 0;
tReadWriteType g_eLEDD1RW = READ_WRITE;

/* Global resource to hold cloud connection states. */
tCloudState g_ui32State = Cloud_Idle;

/* Global resource to indicate the state of the connection to the cloud server. */
bool g_bServerConnect = false;

extern void fdOpenSession();
extern void fdCloseSession();
extern void *TaskSelf();

/* Mailbox to post cloud task messages. */
extern mqd_t               mqCmdQueueBlock;
extern mqd_t               mqCmdQueueNoBlock;
extern mqd_t               mqCloudQueueBlock;
extern mqd_t               mqCloudQueueNoBlock;

/*
 *  ======== ServerConnect ========
 * This function creates a HTTP client instance and connect to the Exosite
 * Server.
 */
int32_t ServerConnect(HTTPClient_Handle * pcli)
{
    int32_t i32Ret = 0;
    int16_t ret;
    int16_t statusCode;
    uint32_t ui32Retry = 0;
    char * pcDebug;
    struct timespec ts;
    HTTPClient_Handle cli;

    /* Initialize the necessary parameters to print over the UART terminal. */
    g_sDebug.ui32Request = Cmd_Prompt_Print;
    pcDebug = g_sDebug.pcBuf;

    /* If proxy settings are needed, set them here. */
#ifdef SET_PROXY
    CloudProxySet(PROXY_ADDR);
#endif

    /* Print message on UART terminal and CCS console, ensuring that the
     *existing command prompt is removed before printing this message (over
     *the UART terminal). */
    g_sDebug.ui32Request = Cmd_Prompt_No_Print;
    snprintf(pcDebug, TX_BUF_SIZE, "Connecting to server...");
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 100000000;
    mq_timedsend(mqCmdQueueBlock, (char*)(&g_sDebug), 0, 0, &ts);
    g_sDebug.ui32Request = Cmd_Prompt_Print;

    /* Try a few times to establish connection with the Exosite server. */
    while(ui32Retry < 5)
    {
        /* Create an HTTP client instance. */
        cli = HTTPClient_create(&statusCode, 0);
        if (statusCode < 0)
        {
            snprintf(pcDebug, TX_BUF_SIZE,"Failed to create http client "
                    "handle");
            mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
            return -1;
        }
        *pcli = cli;

        /* Connect a socket to Exosite server. */
        ret = HTTPClient_connect(cli, g_pcExositeURL, 0, 0);
        if(ret == 0)
        {
            /* Success.  Return from the loop. */
            snprintf(pcDebug, TX_BUF_SIZE, "Connected to Exosite server.\n");
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 100000000;
            mq_timedsend(mqCmdQueueBlock, (char*)(&g_sDebug), 0, 0, &ts);
            return (0);
        }
        else
        {
            /* If we failed to connect, display this information and try for
             * a few more times. */
            g_sDebug.ui32Request = Cmd_Prompt_No_Print;
            snprintf(pcDebug, TX_BUF_SIZE, "Failed to connect to server, "
                     "ecode: %d. Retrying...", (int) i32Ret);
            mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
            g_sDebug.ui32Request = Cmd_Prompt_Print;

            ui32Retry ++;

            /* Deconstruct the HTTP client instance. */
            HTTPClient_destroy(cli);

            /* Sleep for a sec between each trial. */
            sleep(1);
        }
    }

    /* Failed to connect so return with error. */
    snprintf(pcDebug, TX_BUF_SIZE, "Failed to connect to server after 5 "
             "trials.\n");
    mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
    return -1;
}

/*
 *  ======== ServerReconnect ========
 * This function creates a HTTP client instance and connect to the Exosite
 * Server.
 */
int32_t ServerReconnect(HTTPClient_Handle * pcli)
{
    int16_t statusCode = 0;
    int16_t ret = 0;
    uint32_t ui32Retry = 0;
    char * pcDebug;
    HTTPClient_Handle cli;

    /* Initialize the necessary parameters to print over the UART terminal. */
    g_sDebug.ui32Request = Cmd_Prompt_Print;
    pcDebug = g_sDebug.pcBuf;

    /* Try a few times to establish connection with the Exosite server. */
    while(ui32Retry < 5)
    {
        /* Create an HTTP client instance. */
        cli = HTTPClient_create(&statusCode, 0);
        if (statusCode < 0)
        {
            snprintf(pcDebug, TX_BUF_SIZE,"Failed to create http client "
                    "handle");
            mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
            return -1;
        }
        *pcli = cli;

        /* Connect a socket to Exosite server. */
        ret = HTTPClient_connect(cli, g_pcExositeURL, 0, 0);
        if(ret == 0)
        {
            /* Success.  Return from the loop. */
            return (0);
        }

        /* Did not connect.  Try again. */
        ui32Retry ++;

        /* Deconstruct the HTTP client instance. */
        HTTPClient_destroy(cli);

        /* Sleep for a sec between each trial. */
        sleep(1);
    }

    /* Failed to connect so return with error. */
    snprintf(pcDebug, TX_BUF_SIZE, "Failed to connect to server after 5 "
             "trials.\n");
    mq_send(mqCmdQueueNoBlock, (char*)(&pcDebug), 0, 0);
    return -1;
}

/*
 *  ======== ServerDisconnect ========
 * Disconnect from the Exosite Server and destroy the HTTP client instance.
 */
void ServerDisconnect(HTTPClient_Handle * pcli)
{
    HTTPClient_Handle cli;

    cli = *pcli;
    HTTPClient_disconnect(cli);
}

/*
 *  ======== ServerResetSocket ========
 * Disconnect and Connect back to the Exosite Server.
 */
void ServerResetSocket(HTTPClient_Handle * pcli)
{
    ServerDisconnect(pcli);
    while(ServerReconnect(pcli) != 0)
    {
        /* Do nothing. */
    }

    return;
}

/*
 *  ======== GetPID ========
 * This function tries to read a Product ID from non-volatile memory.  If PID
 * is not found in non-volatile memory, then it will prompt user to provide a
 * PID.  Once PID is available, it is stored in a global resource.
 */
bool GetPID(void)
{
    tMailboxMsg sPIDCmdVal;
    char pcPID[EXOSITE_PID_LENGTH + 1];
    char * pcDebug;
    uint32_t ui32Ind;

    /* Initialize the necessary parameters to print over the UART terminal. */
    g_sDebug.ui32Request = Cmd_Prompt_Print;
    pcDebug = g_sDebug.pcBuf;

    /* Try to get the CIK from EERPOM and check if a valid CIK was returned. */
    if(GetPIDEEPROM(pcPID) == true)
    {
        if(pcPID != NULL)
        {
            /* Check if the PID value retrieved from EEPROM is all Fs */
            ui32Ind = 0;
            while(ui32Ind < EXOSITE_PID_LENGTH)
            {
                if(*((uint32_t*) pcPID + ui32Ind) == 0xFFFFFFFF)
                {
                    /* Break on finding the first sequence of 4 consecutive
                     * bytes that are FF. */
                    break;
                }
                ui32Ind++;
            }

            /* Check if we checked most of the bytes of the PID. */
            if(ui32Ind >= ((EXOSITE_PID_LENGTH / 4) - 2))
            {
                /* PID Found. Update the global resource with PID found in
                 * NVM */
                memcpy(g_pcExositePID, pcPID, EXOSITE_PID_LENGTH + 1);

                /* Report this to user */
                snprintf(pcDebug, TX_BUF_SIZE, "PIK found in EEPROM %s\n",
                         g_pcExositePID);
                mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

                /* Return success. */
                return true;
            }
        }
    }

    /* PID not found. Report this. */
    snprintf(pcDebug, TX_BUF_SIZE, "No PID found in EEPROM.\n");
    mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

    /* Prompt user to enter a PID. */
    snprintf(pcDebug, TX_BUF_SIZE, "Provide a PID to proceed.\n  Command: "
             "pid <Your_PID>\n\n");
    mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

    /* Wait for ever till user provides correct product id details. */
    mq_receive(mqCloudQueueBlock, (char*)(&sPIDCmdVal), sizeof(tMailboxMsg), 0);

    /* Copy the received id to a local buffer. */
    strncpy(pcPID, sPIDCmdVal.pcBuf, EXOSITE_PID_LENGTH + 1);

    /* Save the pid to EEPROM */
    if(SavePIDEEPROM(pcPID) == false)
    {
        snprintf(pcDebug, TX_BUF_SIZE,"\nError storing PID in non-volatile "
                 "storage.\r\n");
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
        return(false);
    }

    /* Read back PID from EEPROM. */
    if(GetPIDEEPROM(g_pcExositePID) != true)
    {
        snprintf(pcDebug, TX_BUF_SIZE,"\nError reading back the stored PID "
                 "value.\r\n");
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
        return(false);
    }

    /* Compare the value read back from EEPROM with the value written to it. */
    if(strncmp(g_pcExositePID, pcPID, EXOSITE_PID_LENGTH) != 0)
    {
        /* Values don't match.  Return the error message. */
        snprintf(pcDebug, TX_BUF_SIZE,"\nError the value stored in "
                "non-volatile storage\n\rdoes not match the value sent by user."
                "\r\n");
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
        return(false);
    }

    /* Return success. */
    return true;
}

/*
 *  ======== GetCIK ========
 * This function tries to read a CIK from non-volatile memory and reports if
 * one if found or not.  If a CIK is present in NVM then it is copied to global
 * resource.
 */
bool GetCIK(void)
{
    char pcCIK[EXOSITE_CIK_LENGTH + 1];
    char * pcDebug;
    uint32_t ui32Ind;

    /* Initialize the necessary parameters to print over the UART terminal. */
    g_sDebug.ui32Request = Cmd_Prompt_Print;
    pcDebug = g_sDebug.pcBuf;

    /* Try to get the CIK from EERPOM and check if a valid CIK was returned. */
    if(GetCIKEEPROM(pcCIK) == true)
    {
        if(pcCIK != NULL)
        {
            /* Check if the CIK value retrieved from EEPROM is all Fs */
            ui32Ind = 0;
            while(ui32Ind < EXOSITE_CIK_LENGTH)
            {
                if(*((uint32_t*) pcCIK + ui32Ind) == 0xFFFFFFFF)
                {
                    /* Break on finding the first sequence of 4 consecutive
                     * bytes that are FF. */
                    break;
                }
                ui32Ind++;
            }

            /* Check if we checked most of the bytes of the CIK. */
            if(ui32Ind >= ((EXOSITE_CIK_LENGTH / 4) - 2))
            {
                /* CIK Found.  Update the global resource with CIK found in
                 * NVM. */
                memcpy(g_pcExositeCIK, pcCIK, EXOSITE_CIK_LENGTH + 1);

                /* Report this to user. */
                snprintf(pcDebug, TX_BUF_SIZE, "Token found in EEPROM %s\n",
                         g_pcExositeCIK);
                mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

                /* Return success. */
                return true;
            }
        }
    }

    /* CIK not found. Report this. */
    memset(g_pcExositeCIK, 0, EXOSITE_CIK_LENGTH + 1);
    snprintf(pcDebug, TX_BUF_SIZE, "No Token found in EEPROM.\n");
    mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

    /* Return failure. */
    return false;
}

/*
 *  ======== CloudProxySet ========
 * Set the proxy value if requested by the user.
 */
int32_t CloudProxySet(char *pcProxy)
{
    char * pcPort;
    uint32_t ipAddress;
    uint16_t portNumber;
    char strip[RX_BUF_SIZE];
    SlNetSock_Addr_t *sa;
    SlNetSock_AddrIn_t sAddr;

    /* Check if the buffer has proxy address. */
    if(pcProxy == NULL)
    {
        /* No - Return without doing anything. */
        return -1;
    }

    /* Check if a port number was passed. */
    pcPort = strstr(pcProxy, ":");
    if(pcPort == NULL)
    {
        /* No - Return without doing anything. */
        return -1;
    }

    /* Zero fill a buffer and copy IP address, passed by the user, into it. */
    memset(strip, '\0', sizeof(strip));
    memcpy((void*)strip, (void*)pcProxy, (pcPort - pcProxy));

    /* Convert the port number, passed by the user, into an integer. */
    pcPort++;
    portNumber = atoi(pcPort);

    /* Transform address string into binary */
    SlNetUtil_inetPton(SLNETSOCK_AF_INET, strip, &ipAddress);

    /* Enter the proxy IP address and port number so that it can be passed to
     * the HTTP Client module*/
    sAddr.sin_family = SLNETSOCK_AF_INET;
    sAddr.sin_port = SlNetUtil_htons((unsigned short)portNumber);
    sAddr.sin_addr.s_addr = (unsigned int)ipAddress;
    sa = (SlNetSock_Addr_t*)&sAddr;

    /* HTTPClient_setProxy() expects a SlNetSock_Addr_t, but the input is
     * treated like a SlNetSock_AddrIn_t when the socket is created */
    HTTPClient_setProxy(sa);

    return 0;
}

/*
 *  ======== ReadOnTime ========
 * Calculates and returns time since reset.  Note that this calculation is not
 * very accurate.
 */
uint32_t ReadOnTime(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec);
}

/*
 *  ======== GetRequestBody ========
 * Builds the Request Body for the POST request.
 */
void GetRequestBody(char *pcDataBuf, uint32_t ui32DataBufLen)
{
    uint16_t ui16Temp = 0;
    uint32_t pui32Buttons[2] = {0, 0};
    uint32_t ui32DataLen = 0;
    uint32_t ui32OnTime = 0;

    ui16Temp = ReadInternalTemp();
    ReadButtons(pui32Buttons);
    ui32OnTime = ReadOnTime();
    ui32DataLen = snprintf(pcDataBuf, ui32DataBufLen, "usrsw1=%d&usrsw2=%d",
                           (unsigned int) pui32Buttons[0],
                           (unsigned int) pui32Buttons[1]);
    ui32DataLen += snprintf((pcDataBuf + ui32DataLen),
                            (ui32DataBufLen - ui32DataLen), "&temp=%u",
                            ui16Temp);
    ui32DataLen += snprintf((pcDataBuf + ui32DataLen),
                            (ui32DataBufLen - ui32DataLen), "&ontime=%d",
                            (unsigned int) ui32OnTime);
    if((g_eLEDD1RW == READ_WRITE) || (g_eLEDD1RW == WRITE_ONLY))
    {
        ui32DataLen += snprintf((pcDataBuf + ui32DataLen),
                                (ui32DataBufLen - ui32DataLen), "&led=%d",
                                (unsigned int) g_ui32LEDD1);
    }
    if(g_eLEDD1RW == READ_WRITE)
    {
        g_eLEDD1RW = READ_ONLY;
    }
}

/*
 *  ======== GetAliasList ========
 * Builds the Alias List that can be sent with the POST request.
 */
void GetAliasList(char *pcDataBuf, uint32_t ui32DataBufLen)
{
    uint32_t ui32DataLen = 0;

    ui32DataLen += snprintf(pcDataBuf, ui32DataBufLen, "?");
    if(g_eLEDD1RW == READ_ONLY)
    {
        ui32DataLen += snprintf((pcDataBuf + ui32DataLen),
                                (ui32DataBufLen - ui32DataLen), "led");
    }
}

/*
 *  ======== UpdateCloudData ========
 * Update the state of different board resources with data received from the
 * cloud server.  Currently only the state of the LED D1 gets updated.
 */
void UpdateCloudData(void)
{
    /* Check if the LED state needs to be updated. */
    if(g_ui32LEDD1 != g_ui32LastLEDD1)
    {
        g_ui32LastLEDD1 = g_ui32LEDD1;
        if(g_ui32LEDD1)
        {
            GPIO_write(CONFIG_GPIO_LED0, CONFIG_GPIO_LED_ON);
        }
        else
        {
            GPIO_write(CONFIG_GPIO_LED0, CONFIG_GPIO_LED_OFF);
        }
    }
}

/*
 *  ======== ProcessResponseBody ========
 * Handles the response Body for the GET request.
 */
void ProcessResponseBody(char *pcBuf)
{
    uint32_t ui32Index, j;
    char *pcValueStart = NULL;
    char ppcValueBuf[ALIAS_PROCESSING][VALUEBUF_SIZE];

    /* Get the data for each alias, from the response body to the GET request. */
    for(ui32Index = 0; ui32Index < ALIAS_PROCESSING; ui32Index++)
    {
        /* Search the alias in the buffer. */
        pcValueStart = strstr(pcBuf, g_ppcGETAlias[ui32Index]);

        /* If we could not find the alias in the buffer, continue to the next
         * value. */
        if(!pcValueStart)
        {
            continue;
        }

        /* Find the equals-sign, which should be just before the start of the
         * value. */
        pcValueStart = strstr(pcValueStart, "=");

        /* If we could not find the equals-sign in the buffer, continue to the
         * next value. */
        if(!pcValueStart)
        {
            continue;
        }

        /* Advance to the first character of the value. */
        pcValueStart++;

        /* Loop through the buffer to reach the end of the input value and copy
         * characters to the destination string. */
        j = 0;
        while(j < VALUEBUF_SIZE)
        {
            /* Check for the end of the value string. */
            if((pcValueStart[j] == '&') ||
               (pcValueStart[j] == 0))
            {
                /* Yes - We have reached the end of the value, null-terminate
                 * the destination string, and return. */
                ppcValueBuf[ui32Index][j] = 0;
                break;
            }
            else
            {
                /* No - Copy this byte. */
                ppcValueBuf[ui32Index][j] = pcValueStart[j];
            }

            j++;
        }
    }
    for(ui32Index = 0; ui32Index < ALIAS_PROCESSING; ui32Index++)
    {
        if(strncmp(g_ppcGETAlias[ui32Index], "led", 15) == 0)
        {
            if(g_eLEDD1RW == READ_ONLY)
            {
                g_ui32LEDD1 = (ppcValueBuf[ui32Index][0] == '1') ? 1 : 0;
            }
        }
    }
}

/*
 *  ======== CloudHandleError ========
 * This function handles the server reported error message from functions like
 * ExositeActivate, ExositeWrite and ExositeRead.
 */
void CloudHandleError(int16_t ret, tCloudState *pui32State)
{
    char * pcDebug;

    g_sDebug.ui32Request = Cmd_Prompt_Print;
    pcDebug = g_sDebug.pcBuf;

    /* Check if error handling is needed. */
    if(ret <= 0)
    {
        /* No - Then retun without doing anything. */
        return;
    }

    /* Check if we got a 409 (Conflict) or 404 (Not Found) error. */
    if((ret == HTTP_SC_CONFLICT) || (ret == HTTP_SC_NOT_FOUND))
    {
        /* Yes - Print the reason for this message and try after 10 secs to get
         * a new CIK. */
        snprintf(pcDebug, TX_BUF_SIZE, "CloudError: Server sent %s error."
                 "\n    Check if board is added to the Exosite server.\n",
                 ((ret == HTTP_SC_CONFLICT) ? "409" : "404"));
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

        snprintf(pcDebug, TX_BUF_SIZE, "    Re-enable the device for "
                 "provisioning on Exosite server.\n");
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
        if(g_pcExositeCIK[0] == '\0')
        {
            snprintf(pcDebug, TX_BUF_SIZE, "    Copy a temporary token from "
                     "Exosite server to the LaunchPad using the command "
                     "\"Token\".\n");
            mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
        }
        snprintf(pcDebug, TX_BUF_SIZE, "    Retrying in 10 secs...\n");
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

        /* Sleep for 10 secs and set flag to request a new CIK. */
        sleep(10);
        *pui32State = Cloud_Activate_CIK;
    }

    /* Check if we got a 401 Not Found error. */
    else if(ret == HTTP_SC_UNAUTHORIZED)
    {
        /* No or invalid CIK used during communication with server.
         * Hence should try to get a valid CIK from server. */
        memset(g_pcExositeCIK, 0, EXOSITE_CIK_LENGTH + 1);
        snprintf(pcDebug, TX_BUF_SIZE, "CloudError: Server sent 401 error."
                 "\n    Invalid token used. Trying to acquire a new token.\n");
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

        /* Sleep for 10 secs and set flag to request a new CIK. */
        sleep(10);
        *pui32State = Cloud_Activate_CIK;
    }

    /* We got some unknown error from server. */
    else
    {
        /* Report error and try again. */
        snprintf(pcDebug, TX_BUF_SIZE, "CloudError: Server returned : %d "
                 "during : %d action. Retrying\n", (int) ret, *pui32State);
        mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
    }
}

/*
 *  ======== ExositeActivate ========
 * Get token from Exosite.  The following headers and request body
 * are sent by this function.
 *
 * POST /provision/activate HTTP/1.1
 * Content-Type: application/x-www-form-urlencoded; charset=utf-8
 * Host: <pid>.devmode-m2.exosite.com
 * Content-Length: <length>
 *
 * id=<MC_Address>
 */
int32_t ExositeActivate(HTTPClient_Handle *pcli)
{
    char pcExositeProvBuf[EXOSITE_LENGTH];
    bool bMoreFlag;
    int16_t ret = 0;
    uint32_t ui32Status = 0;
    HTTPClient_Handle cli;

    cli = *pcli;

    /* Assemble the provisioning information. */
    snprintf(pcExositeProvBuf, EXOSITE_LENGTH, "id=%s", g_pcMACAddress);

    /* Send content type header.
     * Content-Type: application/x-www-form-urlencoded; charset=utf-8 */
    ret = HTTPClient_setHeader(cli, HTTPClient_HFIELD_REQ_CONTENT_TYPE,
                               EXOSITE_CONTENT_TYPE,
                               strlen(EXOSITE_CONTENT_TYPE),
                               HTTPClient_HFIELD_PERSISTENT);
    if(ret < 0)
    {
        return (ret);
    }

    /* Send X-Exosite-CIK header, if re-provisioning.
     * X-Exosite-CIK: <CIK>*/
    if(CheckBufZero(g_pcExositeCIK, EXOSITE_CIK_LENGTH) != 0)
    {
        ret = HTTPClient_setHeaderByName(cli, HTTPClient_REQUEST_HEADER_MASK,
                                         EXOSITE_TYPE, (void *)g_pcExositeCIK,
                                         strlen(g_pcExositeCIK),
                                         HTTPClient_HFIELD_PERSISTENT);
        if(ret < 0)
        {
            return (ret);
        }
    }

    /* Make HTTP 1.1 POST request.
     * POST /provision/activate HTTP/1.1
     *
     * The following headers are automatically sent with the POST request
     * Host: <pid>.devmode-m2.exosite.com
     * Content-Length: <length>
     *
     * The request body stored in pcExositeProvBuf is also sent with this API.
     * id=<MC_Address> */
    ret = HTTPClient_sendRequest(cli, HTTP_METHOD_POST, PROVISION_URI,
                                 pcExositeProvBuf, strlen(pcExositeProvBuf),
                                 0);
    if(ret < 0)
    {
        return (ret);
    }

    /* The above API returns the status returned by the Server. */
    ui32Status = (uint32_t)(ret);

    /* Did Exosite respond with a status other than HTTP_NO_CONTENT? */
    if(ui32Status != HTTP_SC_NO_CONTENT)
    {
        /* Yes - Then extract the Response body into a buffer. */
        do
        {
            ret = HTTPClient_readResponseBody(cli, pcExositeProvBuf,
                                              sizeof(pcExositeProvBuf),
                                              &bMoreFlag);
            if(ret < 0)
            {
                return (ret);
            }
            else if(ret)
            {
                if(ret < sizeof(pcExositeProvBuf))
                {
                    pcExositeProvBuf[ret] = '\0';
                }
            }
        } while (bMoreFlag);
    }

    /* Did Exosite respond with a status other than HTTP_OK? */
    if(ui32Status != HTTP_SC_OK)
    {
        /* Yes - Return with the response status. */
        return (ui32Status);
    }

    /* The response body is the CIK.  Save it in EEPROM for future use. */
    SaveCIKEEPROM(pcExositeProvBuf);

    /* Read back the CIK from EEPROM into global resource. */
    if(GetCIKEEPROM(g_pcExositeCIK) != true)
    {
        /* Error reading back CIK.  Return the error message. */
        return(-3);
    }

    /* Compare the value read back from EEPROM with the value received from
     * Exosite. */
    if(strncmp(g_pcExositeCIK, pcExositeProvBuf, EXOSITE_CIK_LENGTH) != 0)
    {
        /* Values don't match.  Return the error message. */
        return(-4);
    }

    /* Print CIK value. */
    g_sDebug.ui32Request = Cmd_Prompt_Print;
    snprintf(g_sDebug.pcBuf, TX_BUF_SIZE, "Token acquired: %s\r\n",
             g_pcExositeCIK);
    mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

    /* No error encountered.  Return success. */
    return(0);
}

/*
 *  ======== ExositeWrite ========
 * Writes (or POSTs) data to Exosite.  The following headers and request body
 * are sent by this function.
 *
 * POST /onep:v1/stack/alias HTTP/1.1
 * X-Exosite-CIK: <CIK>
 * Content-Type: application/x-www-form-urlencoded; charset=utf-8
 * Host: <pid>.devmode-m2.exosite.com
 * Content-Length: <length>
 *
 * <alias 1>=<value 1>&<alias 2...>=<value 2...>&<alias n>=<value n>
 */
int32_t ExositeWrite(HTTPClient_Handle *pcli)
{
    int16_t ret = 0;
    uint32_t ui32Status = 0;
    char pcDataBuf[128];
    bool bMoreFlag;
    HTTPClient_Handle cli;

    cli = *pcli;

    /* Make sure that CIK is filled before proceeding. */
    if(g_pcExositeCIK[0] == '\0')
    {
        /* CIK is not populated.  Return this error. */
        return -1;
    }

    /* Fill-up the request body and the content length. */
    GetRequestBody(pcDataBuf, sizeof(pcDataBuf));

    /* Set content type header
     * Content-Type: application/x-www-form-urlencoded; charset=utf-8 */
    ret = HTTPClient_setHeader(cli, HTTPClient_HFIELD_REQ_CONTENT_TYPE,
                               EXOSITE_CONTENT_TYPE,
                               strlen(EXOSITE_CONTENT_TYPE) + 1,
                               HTTPClient_HFIELD_PERSISTENT);
    if(ret < 0)
    {
        return (ret);
    }

    /* Set X-Exosite-CIK header
     * X-Exosite-CIK: <CIK> */
    ret = HTTPClient_setHeaderByName(cli, HTTPClient_REQUEST_HEADER_MASK,
                                     EXOSITE_TYPE, (void *)g_pcExositeCIK,
                                     strlen(g_pcExositeCIK) + 1,
                                     HTTPClient_HFIELD_PERSISTENT);
    if(ret < 0)
    {
        return (ret);
    }

    /* Make HTTP 1.1 POST request.
     * POST /onep:v1/stack/alias HTTP/1.1
     *
     * The following headers are automatically sent with the POST request
     * Host: <pid>.devmode-m2.exosite.com
     * Content-Length: <length>
     *
     * The request body stored in pcDataBuf is also sent with this API.
     * <alias 1>=<value 1>&<alias 2...>=<value 2...>&<alias n>=<value n> */
    ret = HTTPClient_sendRequest(cli, HTTP_METHOD_POST, EXOSITE_URI,
                                 pcDataBuf, strlen(pcDataBuf), 0);
    if(ret < 0)
    {
        return (ret);
    }

    /* The above API returns the status returned by the Server. */
    ui32Status = (uint32_t)(ret);

    /* Did Exosite respond with an undesired status? */
    if(ui32Status != HTTP_SC_NO_CONTENT)
    {
        /* Yes - Now extract the Response body into a buffer.  Even though we
         * don't need this response body, we need to clean up for next HTTP
         * request. */
        do
        {
            ret = HTTPClient_readResponseBody(cli, pcDataBuf,
                                              sizeof(pcDataBuf), &bMoreFlag);
            if(ret < 0)
            {
                return (ret);
            }
            else if(ret)
            {
                if(ret < sizeof(pcDataBuf))
                {
                    pcDataBuf[ret] = '\0';
                }
            }
        } while (bMoreFlag);

        /* Yes - Return the response status. */
        return (ui32Status);
    }

    /* Received the desired response status from server.  Hence return 0. */
    return(0);
}

/*
 *  ======== ExositeRead ========
 * Reads (or GETs) data from Exosite.  The following headers are sent by this
 * function.
 *
 * GET /onep:v1/stack/alias?ledd1&ledd2&location HTTP/1.1
 * Host: <pid>.devmode-m2.exosite.com
 * X-Exosite-CIK: <CIK>
 * Accept: application/x-www-form-urlencoded; charset=utf-8
 */
int32_t ExositeRead(HTTPClient_Handle *pcli)
{
    int16_t ret = 0;
    uint32_t ui32Status = 0;
    uint32_t ui32BufLen = 0;
    char pcRecBuf[128];
    bool bMoreFlag;
    HTTPClient_Handle cli;

    cli = *pcli;

    /* Make sure that CIK is filled before proceeding. */
    if(g_pcExositeCIK[0] == '\0')
    {
        /* CIK is not populated.  Return this error. */
        return -1;
    }

    /* Copy Exosite URI into a buffer. */
    ui32BufLen = snprintf(pcRecBuf, sizeof(pcRecBuf), EXOSITE_URI);

    /* Get the alias list whose values we need from the cloud server. */
    GetAliasList((pcRecBuf + ui32BufLen), (sizeof(pcRecBuf) - ui32BufLen));

    /* Send X-Exosite-CIK header
     * X-Exosite-CIK: <CIK> */
    ret = HTTPClient_setHeaderByName(cli, HTTPClient_REQUEST_HEADER_MASK,
                                     EXOSITE_TYPE, (void *)g_pcExositeCIK,
                                     strlen(g_pcExositeCIK) + 1,
                                     HTTPClient_HFIELD_PERSISTENT);
    if(ret < 0)
    {
        return (ret);
    }

    /* Send accept header
     * Accept: <type> */
    ret = HTTPClient_setHeader(cli, HTTPClient_HFIELD_REQ_ACCEPT,
                               EXOSITE_CONTENT_TYPE,
                               strlen(EXOSITE_CONTENT_TYPE) + 1,
                               HTTPClient_HFIELD_PERSISTENT);
    if(ret < 0)
    {
        return (ret);
    }

    /* Make HTTP 1.1 GET request.
     * GET /onep:v1/stack/alias?ledd1 HTTP/1.1
     *
     * The following headers are automatically sent with the GET request.
     * Host: m2.exosite.com */
    ret = HTTPClient_sendRequest(cli, HTTP_METHOD_GET, pcRecBuf,
                                 NULL, 0, 0);
    if(ret < 0)
    {
        return (ret);
    }

    /* The above API returns the status returned by the Server. */
    ui32Status = ret;

    /* Did Exosite respond with a NO Content response? */
    if(ui32Status == HTTP_SC_NO_CONTENT)
    {
        /* Yes - Return with 0. */
        return (0);
    }

    /* Did Exosite respond with an undesired response? */
    if(ui32Status != HTTP_SC_OK)
    {
        /* Yes - Return with the response status. */
        return (ui32Status);
    }

    /* Extract the Response body into a buffer. */
    do
    {
        ret = HTTPClient_readResponseBody(cli, pcRecBuf, sizeof(pcRecBuf),
                                          &bMoreFlag);
        if(ret < 0)
        {
            return (ret);
        }
        else if(ret)
        {
            if(ret < sizeof(pcRecBuf))
            {
                pcRecBuf[ret] = '\0';
            }
        }
    } while (bMoreFlag);

    /* Parse the response body and perform necessary actions based on the
     * content. */
    ProcessResponseBody(pcRecBuf);

    /* Received the desired response status from server.  Hence return 0. */
    return(0);
}

/*
 *  ======== cloudTask ========
 * This task is the main task that runs the interface to cloud for this app.
 * It also manages the LEDs and Buttons for the board.
 */
void *cloudTask(void *arg0)
{
    int32_t i32Ret = 0;
    HTTPClient_Handle cli;
    tMailboxMsg sCommandRequest;
    char * pcDebug;
    uint32_t ui32LED2;
    int ret;

    fdOpenSession(TaskSelf());

    g_sDebug.ui32Request = Cmd_Prompt_Print;
    pcDebug = g_sDebug.pcBuf;
    ui32LED2 = CONFIG_GPIO_LED_OFF;

    /* Get the PID from EEPROM and check if it is valid. */
    if(GetPID() == false)
    {
        /* Unsuccessful in getting PID.  Jump to Idle state and wait for user
         * command. */
        g_ui32State = Cloud_Idle;
    }
    else
    {
        snprintf(g_pcExositeURL, EXOSITE_URL_LEN, "%s.devmode-%s",
                 g_pcExositePID, EXOSITE_HOSTNAME);

        /* Got PID.  Next attempt to connect to the server. */
        g_ui32State = Cloud_Server_Connect;
    }

    while (1)
    {
        /* Check if we received any notification from the Command task. */
        ret = mq_receive(mqCloudQueueNoBlock, (char*)(&sCommandRequest),
                         sizeof(tMailboxMsg), 0);
        if(ret != -1)
        {
            g_ui32State = (tCloudState) (sCommandRequest.ui32Request);
        }

        /* Update the different board resources. */
        UpdateCloudData();

        switch(g_ui32State)
        {
            case Cloud_Server_Connect:
            {
                if(g_bServerConnect == true)
                {
                    /* If already connected, disconnect before trying to
                     * reconnect. */
                    ServerDisconnect(&cli);
                    g_bServerConnect = false;
                }

                /* Create a secure socket and try to connect with the cloud
                 * server. */
                if(ServerConnect(&cli) != 0)
                {
                    /* Unsuccessful in connecting to the server.  Jump to Idle
                     * state and wait for user command. */
                    g_ui32State = Cloud_Idle;
                }
                else
                {
                    /* Success.  Set the global resource to indicate connection
                     * to cloud server. */
                    g_bServerConnect = true;

                    /* See if a valid CIK exists in NVM. */
                    if(GetCIK() == true)
                    {
                        /* Yes - Try to send information to the server with
                         * this CIK. */
                        g_ui32State = Cloud_Write;
                    }
                    else
                    {
                        /* No - Get a new CIK. */
                        g_ui32State = Cloud_Activate_CIK;
                    }
                }

                break;
            }

            case Cloud_Activate_CIK:
            {
                /* If we are not yet connected to the cloud server, break and
                 * connect to server first. */
                if(g_bServerConnect != true)
                {
                    g_ui32State = Cloud_Server_Connect;
                    break;
                }

                /* We don't have a valid CIK.  We would have tried to connect
                 * to server with an invalid CIK, so reconnect to the server
                 * before trying to request a new CIK. */
                ServerResetSocket(&cli);

                /* Request a new CIK. */
                i32Ret = ExositeActivate(&cli);
                if(i32Ret == 0)
                {
                    /* We acquired new CIK so communicate with the server. */
                    g_ui32State = Cloud_Write;
                }

                break;
            }

            case Cloud_Write:
            {
                /* If we are not yet connected to the cloud server, break and
                 * connect to server first. */
                if(g_bServerConnect != true)
                {
                    g_ui32State = Cloud_Server_Connect;
                    break;
                }

                /* Gather and send relevant data to Exosite server. */
                i32Ret = ExositeWrite(&cli);
                if(i32Ret != 0)
                {
                    /* We got an error, so break to handle error. */
                    break;
                }

                /* Blink LED to indicate that communication is occurring. */
                ui32LED2 ^= CONFIG_GPIO_LED_ON;
                GPIO_write(CONFIG_GPIO_LED1, ui32LED2);

                /* We were successful in communicating with cloud server.
                 * Continue to do this. */
                ServerResetSocket(&cli);

                g_ui32State = Cloud_Read;
                break;
            }

            case Cloud_Read:
            {
                /* If we are not yet connected to the cloud server, break and
                 * connect to server first. */
                if(g_bServerConnect != true)
                {
                    g_ui32State = Cloud_Server_Connect;
                    break;
                }

                /* Read data from Exosite server and process it. */
                i32Ret = ExositeRead(&cli);
                if(i32Ret != 0)
                {
                    /* We got an error, so break to handle error. */
                    break;
                }

                /* Blink LED to indicate that communication is occurring. */
                ui32LED2 ^= CONFIG_GPIO_LED_ON;
                GPIO_write(CONFIG_GPIO_LED1, ui32LED2);

                /* We were successful in communicating with cloud server.
                 * Continue to do this. */
                ServerResetSocket(&cli);

                g_ui32State = Cloud_Write;
                break;
            }

            case Cloud_Proxy_Set:
            {
                /* Set state machine state to connect to server and update IP
                 * address and proxy state */
                i32Ret = CloudProxySet(sCommandRequest.pcBuf);
                if(i32Ret != 0)
                {
                    /* Error - Go back to idle state */
                    g_ui32State = Cloud_Idle;
                    break;
                }
                else
                {
                    /* Proxy has been set. Now attempt to connect to server. */
                    g_ui32State = Cloud_Server_Connect;
                }
                break;
            }

            case Cloud_Update_Token:
            {
                /* Set state machine state to connect to server and update IP
                 * address and proxy state */
                strncpy(g_pcExositeCIK, sCommandRequest.pcBuf,
                        EXOSITE_CIK_LENGTH + 1);
                g_ui32State = Cloud_Activate_CIK;
                break;
            }

            case Cloud_Update_PID:
            {
                /* Send debug message. */
                snprintf(pcDebug, TX_BUF_SIZE, "This command is supported "
                         "only during initialization.\n");
                mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);
                g_ui32State = Cloud_Write;
                break;
            }

            case Cloud_Idle:
            {
                /* Clear errors and do nothing. */
                i32Ret = 0;

                /* Wait a few moments before proceeding. */
                usleep(100000);

                break;
            }

            default:
            {
                /* This case should never occur.  Send debug message. */
                snprintf(pcDebug, TX_BUF_SIZE, "CloudTask: Default case "
                         "should never occur. Ecode: %d.\n    Retrying "
                         "connection with server.\n", (int) i32Ret);
                mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

                /* Set the state variable to retry connection. */
                g_ui32State = Cloud_Server_Connect;
                i32Ret = 0;
                break;
            }
        }

        /* Have we been trying to connect to the cloud server? */
        if((g_ui32State == Cloud_Activate_CIK) ||
           (g_ui32State == Cloud_Write) || (g_ui32State == Cloud_Read))
        {
            /* Yes - Check if we got a -ve error. */
            if(i32Ret < 0)
            {
                /* This means the HTTPCli module cannot be recovered.  Report
                 * the error and reset cloud connection to try again. */
                snprintf(pcDebug, TX_BUF_SIZE, "CloudTask: Bad response,"
                         " ecode: %d during : %d action.\n    Resetting"
                         " connection.\n", (int) i32Ret, g_ui32State);
                mq_send(mqCmdQueueNoBlock, (char*)(&g_sDebug), 0, 0);

                /* Set state variable to reconnect to cloud server. */
                g_ui32State = Cloud_Server_Connect;
            }

            /* Handle all other errors. */
            else if(i32Ret > 0)
            {
                CloudHandleError(i32Ret, &g_ui32State);
            }

            i32Ret = 0;

            /* Wait a second before communicating with Exosite server again. */
            sleep(1);
        }
    }
}
