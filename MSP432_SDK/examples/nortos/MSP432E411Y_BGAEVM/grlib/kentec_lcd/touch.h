/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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

#ifndef __DRIVERS_TOUCH_H__
#define __DRIVERS_TOUCH_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern volatile int16_t touchX;
extern volatile int16_t touchY;
extern int16_t touchMin;
extern void TouchScreenIntHandler(void);
extern void TouchScreenInit(uint32_t sysClock);
extern void TouchScreenCallbackSet(int32_t (*callbackFxn)(uint32_t message,
    int32_t x, int32_t y));

/*
 * This driver operates in four different screen orientations.  They are:
 *
 * * Portrait - The screen is taller than it is wide, and the flex connector is
 *              on the left of the display.  This is selected by defining
 *              PORTRAIT.
 *
 * * Landscape - The screen is wider than it is tall, and the flex connector is
 *               on the bottom of the display.  This is selected by defining
 *               LANDSCAPE.
 *
 * * Portrait flip - The screen is taller than it is wide, and the flex
 *                   connector is on the right of the display.  This is
 *                   selected by defining PORTRAIT_FLIP.
 *
 * * Landscape flip - The screen is wider than it is tall, and the flex
 *                    connector is on the top of the display.  This is
 *                    selected by defining LANDSCAPE_FLIP.
 *
 * These can also be imagined in terms of screen rotation; if portrait mode is
 * 0 degrees of screen rotation, landscape is 90 degrees of counter-clockwise
 * rotation, portrait flip is 180 degrees of rotation, and landscape flip is
 * 270 degress of counter-clockwise rotation.
 *
 * If no screen orientation is selected, landscape mode will be used.
 *
 */
#if ! defined(PORTRAIT) && ! defined(PORTRAIT_FLIP) &&  \
    ! defined(LANDSCAPE) && ! defined(LANDSCAPE_FLIP)
#define LANDSCAPE
#endif

/* The GPIO pins/ADC channels to which the touch screen is connected. */
#define TS_XP_BASE              GPIO_PORTP_BASE
#define TS_XP_PIN               GPIO_PIN_6
#define TS_XP_ADC               ADC_CTL_CH23
#define TS_XN_BASE              GPIO_PORTP_BASE
#define TS_XN_PIN               GPIO_PIN_7
#define TS_YP_BASE              GPIO_PORTE_BASE
#define TS_YP_PIN               GPIO_PIN_6
#define TS_YP_ADC               ADC_CTL_CH20
#define TS_YN_BASE              GPIO_PORTE_BASE
#define TS_YN_PIN               GPIO_PIN_7

/* The lowest ADC reading assumed to represent a press on the screen.  Readings
   below this indicate no press is taking place. */
#define TOUCH_MIN               150

#define TS_STATE_INIT           0
#define TS_STATE_SKIP_X         1
#define TS_STATE_READ_X         2
#define TS_STATE_SKIP_Y         3
#define TS_STATE_READ_Y         4

#define MSG_PTR_DOWN     0x00000002
#define MSG_PTR_MOVE     0x00000003
#define MSG_PTR_UP       0x00000004

#ifdef __cplusplus
}
#endif

#endif // __DRIVERS_TOUCH_H__
