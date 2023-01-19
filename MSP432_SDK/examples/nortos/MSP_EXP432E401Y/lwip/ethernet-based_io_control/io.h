//*****************************************************************************
//
// io.h - Prototypes for I/O routines for the enet_io example.
//
// Copyright (c) 2009 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************

#ifndef __IO_H__
#define __IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Exported global variables.
//
//*****************************************************************************
extern volatile unsigned long g_ulAnimSpeed;

//*****************************************************************************
//
// Exported function prototypes.
//
//*****************************************************************************
void io_init(void);
void io_set_led(bool bOn);
void io_get_ledstate(char *pcBuf, int iBufLen);
void io_set_animation_speed_string(char *pcBuf);
void io_get_animation_speed_string(char *pcBuf, int iBufLen);
void io_set_animation_speed(unsigned long ulSpeedPercent);
unsigned long io_get_animation_speed(void);
int io_is_led_on(void);

#ifdef __cplusplus
}
#endif

#endif // __IO_H__
