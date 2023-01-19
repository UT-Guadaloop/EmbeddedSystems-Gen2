//*****************************************************************************
//
// cgifuncs.c - Helper functions related to CGI script parameter parsing.
//
// Copyright (c) 2008 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __CGIFUNCS_H__
#define __CGIFUNCS_H__

//*****************************************************************************
//
// Prototypes of functions exported by this module.
//
//*****************************************************************************
int32_t FindCGIParameter(const char *pcToFind, char *pcParam[],
                         int32_t i32NumParams);
bool IsValidHexDigit(const char cDigit);
unsigned char HexDigit(const char cDigit);
bool DecodeHexEscape(const char *pcEncoded, char *pcDecoded);
uint32_t EncodeFormString(const char *pcDecoded, char *pcEncoded,
                               uint32_t ui32Len);
uint32_t DecodeFormString(const  char *pcEncoded, char *pcDecoded,
                               uint32_t ui32Len);
bool CheckDecimalParam(const char *pcValue, int32_t *pi32Value);
int32_t GetCGIParam(const char *pcName, char *pcParams[], char *pcValue[],
                 int32_t i32NumParams, bool *pbError);

#endif // __CGIFUNCS_H__
