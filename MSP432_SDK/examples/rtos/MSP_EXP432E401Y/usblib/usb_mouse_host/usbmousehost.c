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
 *  ======== usbmousehost.c ========
 */
#include <stdbool.h>

/* For usleep() */
#include <unistd.h>

/* Driver Header files */
#include <ti/display/Display.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "ti_drivers_config.h"
#include "ti_usblib_config.h"

/* USB Reference Module Header file */
#include "USBMH.h"

/*
 *  ======== taskFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
void *mouseHostFxn(void *arg0)
{
    USBMH_State state;
    uint32_t time = 100;

    Display_Handle displayHandle;

    Display_init();

    displayHandle = Display_open(Display_Type_UART, NULL);
    if (displayHandle == NULL) {
        /* Display_open() failed */
        while (1);
    }

    Display_printf(displayHandle, 0, 0, "\n");
    Display_printf(displayHandle, 0, 0, "Mouse cursor position\n");

    while (true) {

        /* Block while the device is NOT connected to the USB */
        USBMH_waitForConnect(WAIT_FOREVER);

        /* Determine the status of the mouse */
        USBMH_getState(&state);

        /* Update LED outputs */
        GPIO_write(CONFIG_LED_0, state.button1 ? CONFIG_LED_ON : CONFIG_LED_OFF);
        GPIO_write(CONFIG_LED_1, state.button2 ? CONFIG_LED_ON : CONFIG_LED_OFF);

        Display_printf(displayHandle, 0, 0, "X:%03d\tY:%03d\r", state.deltaX, state.deltaY);

        /* Send data periodically */
        usleep(time);
    }
}
