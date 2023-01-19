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

/*
 *  ======== display.c ========
 */
#include <stdint.h>
#include "touch.h"
#include "kentec.h"
#include <ti/devices/msp432e4/driverlib/sysctl.h>

/* Example GrLib image */
#include "splash_image.h"

extern const Graphics_Display_Functions Kentec_fxns;
extern Graphics_Display Kentec_GD;

Graphics_Context sContext;
Graphics_Rectangle red = {30, 170, 80, 220};
Graphics_Rectangle blue = {130, 170, 180, 220};
Graphics_Rectangle green = {230, 170, 280, 220};

/* This function is called whenever the touch screen gets pressed. In this example,
 * the font of "Hello World" changes based on where the screen is touched. */
int32_t touchcallback(uint32_t message, int32_t x,
        int32_t y)
{
    /* Check where the user touched the screen */
    if (y < 220 && y > 170) {
        /* If pressed within the red box, change the text color to red */
        if (x > 30 && x < 80) {
            Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_RED);
            Graphics_drawStringCentered(&sContext, (int8_t *)"Hello World", -1, 160, 75, false);
        }
        /* If pressed within the blue box, change the text color to blue */
        else if (x > 130 && x < 180) {
            Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_BLUE);
            Graphics_drawStringCentered(&sContext, (int8_t *)"Hello World", -1, 160, 75, false);
        }
        /* If pressed within the green box, change the text color to green */
        else if (x > 230 && x < 270) {
            Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_LIME);
            Graphics_drawStringCentered(&sContext, (int8_t *)"Hello World", -1, 160, 75, false);
        }
    }
    /* Otherwise set the text color to white*/
    else {
        Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_WHITE);
        Graphics_drawStringCentered(&sContext, (int8_t *)"Hello World", -1, 160, 75, false);
    }

    return 0;
}

/*
 *  ======== main function ========
 */
int main(void)
{
    uint32_t ui32SysClock;

    /* Run from the PLL at 120 MHz. */
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
        SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    /* Initialize Kentec LCD driver and touch driver */
    Kentec_Init(ui32SysClock);
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(touchcallback);

    Graphics_initContext(&sContext, &Kentec_GD, &Kentec_fxns);

    /* Draw TI logo and wait 1 sec */
    Graphics_drawImage(&sContext, &splashImage, 00, 0);
    SysCtlDelay(ui32SysClock);

    /* Clear display */
    Graphics_clearDisplay(&sContext);
    SysCtlDelay(10000000);
    Graphics_setBackgroundColor(&sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_WHITE);

    /* Set font size and print text */
    Graphics_setFont(&sContext, &g_sFontCmsc12);
    Graphics_drawStringCentered(&sContext, (int8_t *)"Pick a text color:", -1, 155, 145, false);

    Graphics_setFont(&sContext, &g_sFontCmsc40);
    Graphics_drawStringCentered(&sContext, (int8_t *)"Hello World", -1, 160, 75, false);

    /* Draw color boxes*/
    Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_RED);
    Graphics_fillRectangle(&sContext, &red);

    Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_LIME);
    Graphics_fillRectangle(&sContext, &green);

    Graphics_setForegroundColor(&sContext, GRAPHICS_COLOR_BLUE);
    Graphics_fillRectangle(&sContext, &blue);

    while(1);
}
