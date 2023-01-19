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
 *  ======== USBMD.c ========
 */

/* Header files */
#include <ti/display/Display.h>


/* POSIX Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/MutexP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <stdbool.h>

/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdhid.h>
#include <ti/usblib/msp432e4/device/usbdhidmouse.h>

/* Example/Board Header files */
#include "USBMD.h"
#include "ti_usblib_config.h"

typedef uint32_t            USBMDEventType;

/* Typedefs */
typedef volatile enum {
    USBMD_STATE_IDLE = 0,
    USBMD_STATE_SENDING,
    USBMD_STATE_UNCONFIGURED
} USBMD_USBState;

/* Static variables and handles */
static volatile USBMD_USBState state;
static MutexP_Handle mutexMouse;
static MutexP_Handle mutexUSBWait;
static SemaphoreP_Handle semMouse;
static SemaphoreP_Handle semUSBConnected;

/* Function prototypes */
static void USBMD_hwiHandler(uintptr_t arg0);
static int sendState(USBMD_State *mouseState, unsigned int timeout);
static bool waitUntilSent(unsigned int timeout);

/*
 *  ======== cbMouseHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
USBMDEventType cbMouseHandler (void *cbData, USBMDEventType event,
                                      USBMDEventType eventMsg,
                                      void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            state = USBMD_STATE_IDLE;
            SemaphoreP_post(semUSBConnected);
            break;

        case USB_EVENT_DISCONNECTED:
            if (state == USBMD_STATE_SENDING) {
                state = USBMD_STATE_UNCONFIGURED;
                SemaphoreP_post(semMouse);
            }
            else {
                state = USBMD_STATE_UNCONFIGURED;
            }
            break;

        case USB_EVENT_TX_COMPLETE:
            state = USBMD_STATE_IDLE;
            SemaphoreP_post(semMouse);
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== USBMD_hwiHandler ========
 *  This function calls the USB library's device interrupt handler.
 */
static void USBMD_hwiHandler(uintptr_t arg0)
{
    USB0_IRQDeviceHandler();
}

/*
 *  ======== sendState ========
 */
static int sendState(USBMD_State *mouseState, unsigned int timeout)
{
    unsigned int key;
    int retValue;
    unsigned char buttons = 0;

    /* Set the bit packed button values */
    buttons |= (mouseState->button1) ? HID_MOUSE_BUTTON_1 : 0;
    buttons |= (mouseState->button2) ? HID_MOUSE_BUTTON_2 : 0;
    buttons |= (mouseState->button3) ? HID_MOUSE_BUTTON_3 : 0;

    /* Acquire lock */
    key = MutexP_lock(mutexMouse);

    state = USBMD_STATE_SENDING;
    retValue = USBDHIDMouseStateChange((tUSBDHIDMouseDevice *)&mouseDevice,
                                        mouseState->deltaX,
                                        mouseState->deltaY,
                                        buttons);

    retValue = (retValue) ? retValue : !waitUntilSent(timeout);

    /* Release lock */
    MutexP_unlock(mutexMouse, key);

    return (retValue);
}

/*
 *  ======== waitUntilSent ========
 *  Function will determine if the last key press/release was sent to the host
 *
 *  @return             0: Assume that there was an error (likely a disconnect)
 *                      1: Successful
 */
static bool waitUntilSent(unsigned int timeout)
{

    while (state == USBMD_STATE_SENDING) {
        if (SemaphoreP_pend(semMouse, timeout) == SemaphoreP_TIMEOUT) {
            state = USBMD_STATE_UNCONFIGURED;
            return (false);
        }
    }

    return (true);
}

/*
 *  ======== USBMD_init ========
 */
void USBMD_init(bool usbInternal)
{
    HwiP_Handle hwi;
    uint32_t ui32ULPI;
    Display_Handle display;
    uint32_t ui32PLLRate;

    Display_init();

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);

    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }


    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBMD_hwiHandler, NULL);
    if (hwi == NULL) {
        //Can't create USB Hwi
        Display_printf(display, 0, 0, "Can't create USB Hwi.\n");
        while(1);
    }

    /* RTOS primitives */
    semMouse = SemaphoreP_createBinary(0);
    if (semMouse == NULL) {
        //Can't create mouse semaphore
        Display_printf(display, 0, 0, "Can't create mouse semaphore.\n");
        while(1);

    }

    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        //Can't create USB semaphore
        Display_printf(display, 0, 0, "Can't create USB semaphore.\n");
        while(1);
    }

    mutexMouse = MutexP_create(NULL);
    if (mutexMouse == NULL) {
        //Can't create mouse gate
        Display_printf(display, 0, 0, "Can't create mouse gate.\n");
        while(1);
    }

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        //Could not create USB Wait gate
        Display_printf(display, 0, 0, "Could not create USB Wait gate.\n");
        while(1);
    }

    /* State specific variables */
    state = USBMD_STATE_UNCONFIGURED;

    /* Check if the ULPI mode is to be used or not */
    if(!(usbInternal)) {
        ui32ULPI = USBLIB_FEATURE_ULPI_HS;
        USBDCDFeatureSet(0, USBLIB_FEATURE_USBULPI, &ui32ULPI);
    }

    /* Set the USB stack mode to Device mode with VBUS monitoring */
    USBStackModeSet(0, eUSBModeForceDevice, 0);

 
    /*Gets the VCO frequency of 120MHZ */
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &ui32PLLRate);


    /*Set the PLL*/
    USBDCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &ui32PLLRate);
    /*
     * Pass our device information to the USB HID device class driver,
     * initialize the USB controller and connect the device to the bus.
     */
    if (!USBDHIDMouseInit(0, &mouseDevice)) {
        //Error initializing the mouse
        Display_printf(display, 0, 0, "Error initializing the mouse.\n");
        while(1);
    }
    Display_close(display);

}


/*
 *  ======== USBMD_setState ========
 */
unsigned int USBMD_setState(USBMD_State *mouseState, unsigned int timeout)
{
    unsigned int retValue = 0;

    if (state == USBMD_STATE_IDLE) {
        retValue = sendState(mouseState, timeout);
    }

    return (retValue);
}

/*
 *  ======== USBMD_waitForConnect ========
 */
bool USBMD_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);

    if (state == USBMD_STATE_UNCONFIGURED) {
        if (SemaphoreP_pend(semUSBConnected, timeout) == SemaphoreP_TIMEOUT) {
            ret = false;
        }
    }

    MutexP_unlock(mutexUSBWait, key);

    return (ret);
}
