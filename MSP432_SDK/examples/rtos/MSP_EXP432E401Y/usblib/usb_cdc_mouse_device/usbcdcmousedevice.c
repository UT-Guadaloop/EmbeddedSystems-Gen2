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
 *    ======== usbcdcmousedevice.c ========
 */
#include <stdbool.h>
#include <string.h>

#include <ti/display/Display.h>

/* For usleep() */
#include <ti/drivers/dpl/ClockP.h>

/* header file */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "ti_drivers_config.h"
#include "ti_usblib_config.h"

/* USB Reference Module Header file */
#include "USBCDCMOUSE.h"

typedef signed char offsetCoordinates[2];

const offsetCoordinates mouseLookupTable[] = {
    /* X,  Y */
    {  2,  0 },
    {  0,  2 },
    {  2,  0 },
    {  0, -2 },
    { -2,  0 },
    {  0,  2 },
    { -2,  0 },
    {  0, -2 }, /* These offset values move the cursor in a figure 8 pattern */

    /* Termination entry (X == 0) && (Y == 0) */
    {  0,  0 }
};

/*
 *  ======== mouseCursorFxn ========
 */
void *mouseCursorFxn(void *arg0)
{
    USBCDCMOUSE_State state;

    int i;
    int j;

    while (true) {

        /* Block while the device is NOT connected to the USB */
        USBCDCMOUSE_waitForConnect(WAIT_FOREVER);

        /* For each entry in the lookup table */
        for (j = 0; mouseLookupTable[j][0] || mouseLookupTable[j][1]; j++) {

            /* Perform the action i times to make it look gradual */
            for (i = 0; i < 20; i++) {
                state.deltaX = mouseLookupTable[j][0];
                state.deltaY = mouseLookupTable[j][1];
                state.button1 = GPIO_read(CONFIG_BUTTON_0) ? false : true;
                state.button2 = false;
                state.button3 = false;
                USBCDCMOUSE_setState(&state, WAIT_FOREVER);
            }
        }
    }
}

/*
 *  ======== cdcPeriodicFxn ========
 *  Task to transmit serial data.
 *
 *  This task periodically sends data to the USB host once it's connected.
 */
void *cdcPeriodicFxn(void *arg0)
{
    char *text = (char *)arg0;
    /* Sleep time in microseconds */
    uint32_t   time = 2000000;
    while (true) {

        /* Block while the device is NOT connected to the USB */
        USBCDCMOUSE_waitForConnect(WAIT_FOREVER);

        USBCDCMOUSE_sendData(text, strlen(text)+1, WAIT_FOREVER);
        GPIO_toggle(CONFIG_LED_0);

        /* Send data periodically */
        ClockP_usleep(time);
    }
}

/*
 *  ======== cdcRecvFxn ========
 *  Task to receive serial data.
 *
 *  This task will receive data when data is available and block while the
 *  device is not connected to the USB host or if no data was received.
 */
void *cdcRecvFxn(void *arg0)
{
    unsigned int received;
    unsigned char data[32];

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
        USBCDCMOUSE_waitForConnect(WAIT_FOREVER);

        received = USBCDCMOUSE_receiveData(data, 31, WAIT_FOREVER);

        data[received] = '\0';
        GPIO_toggle(CONFIG_LED_1);
        if (received) {
            Display_printf(displayHandle, 0, 0, "Rcvd \"%s\" (%d bytes)\r\n", data, received);
        }

    }
}
