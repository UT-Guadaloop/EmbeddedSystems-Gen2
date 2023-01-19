//*****************************************************************************
//
// json.h - The parsing function headers for JSON data from the web server.
//
// Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __JSON_H__
#define __JSON_H__

#ifndef JSON_H_
#define JSON_H_

#define INVALID_INT             ((int32_t)(0x80000000))

extern int32_t JSONParseCurrent(uint32_t ui32Index,
                                tWeatherReport *psWeatherReport,
                                struct pbuf *psBuf);
extern int32_t JSONParseForecast(uint32_t ui32Index,
                                 tWeatherReport *psWeatherReport,
                                 struct pbuf *psBuf);
//##### INTERNAL BEGIN #####

#ifdef API_TEST
extern int32_t JSONDataTest(void);
#endif
//##### INTERNAL END #####
#endif

#endif
