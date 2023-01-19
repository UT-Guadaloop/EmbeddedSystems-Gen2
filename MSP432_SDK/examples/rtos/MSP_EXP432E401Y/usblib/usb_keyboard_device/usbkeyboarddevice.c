/* --COPYRIGHT--,BSD
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
 * --/COPYRIGHT--*/
/*
 *  ======== usbkeyboarddevice.c ========
 */

#include <stdbool.h>
#include <string.h>

/* For usleep() */
#include <ti/drivers/dpl/ClockP.h>

/* Driver Header files */
#include <ti/display/Display.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "ti_drivers_config.h"

/* USB Reference Module Header file */
#include "USBKBD.h"

/*
 *  ======== taskFxn ========
 */
void *taskFxn (void *arg0)
{
    USBKBD_State state;
    unsigned int currButton;
    unsigned int prevButton = 1;
    int sent;
    uint32_t time = 100;

    char *text = (char *)arg0;
    Display_Handle displayHandle;

    Display_init();

    displayHandle = Display_open(Display_Type_UART, NULL);
    if (displayHandle == NULL) {
        /* Display_open() failed */
        while (1);
    }

    Display_printf(displayHandle, 0, 0, "\n");

    while (true) {

        /* Block while the device is NOT connected to the USB */
        USBKBD_waitForConnect(WAIT_FOREVER);

        /* Determine the status of the keyboard */
        USBKBD_getState(&state);

        /* Update LED outputs */
        GPIO_write(CONFIG_LED_0, state.capsLED ? CONFIG_LED_ON : CONFIG_LED_OFF);
        GPIO_write(CONFIG_LED_1, state.scrollLED ? CONFIG_LED_ON : CONFIG_LED_OFF);

        /*
         * When Board_BUTTON0 transitions from HIGH to LOW, print string to the
         * USB host
         */
        currButton = GPIO_read(CONFIG_BUTTON_0);
        if((currButton == 0) && (prevButton != 0))
        {
            sent = USBKBD_putString(text, strlen(text)+1, WAIT_FOREVER);
            Display_printf(displayHandle, 0, 0, "Sent %d bytes\n", sent);
        }
        prevButton = currButton;

        /* Send data periodically */
        ClockP_usleep(time);
 
    }
}
