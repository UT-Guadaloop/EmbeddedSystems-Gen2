//*****************************************************************************
//
// debug.h -
//
// Copyright (c) 2013 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG_OUTPUT
//#include "ti/devices/msp432e4/driverlib/uart.h"
#include "uartstdio.h"
#define DebugPrintf(...)        UARTprintf(__VA_ARGS__)
#else
#define DebugPrintf(...)
#endif
#else
#define DebugPrintf(...)

#endif
