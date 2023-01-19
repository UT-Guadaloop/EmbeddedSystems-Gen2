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
 *  ======== USBMH.c ========
 */
#include <stdbool.h>

/* For usleep() */
#include <unistd.h>

/* Header files */
#include <ti/display/Display.h>

/* POSIX Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/MutexP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/host/usbhost.h>
#include <ti/usblib/msp432e4/host/usbhhid.h>
#include <ti/usblib/msp432e4/host/usbhhidmouse.h>

/* POSIX Header files */
#include <pthread.h>

/* Example/Board Header files */
#include "USBMH.h"
#include "ti_usblib_config.h"

typedef tUSBHMouse             *USBMHType;
typedef uint32_t                USBMHEventType;
typedef void                    USBMHHandleType;

/* Defines */
#define HCDMEMORYPOOLSIZE   128 /* Memory for the Host Class Driver */
#define MMEMORYPOOLSIZE     128 /* Memory for the mouse host driver */

/* Typedefs */
typedef volatile enum {
    USBMH_NO_DEVICE = 0,
    USBMH_INIT,
    USBMH_CONNECTED,
    USBMH_UNKNOWN,
    USBMH_POWER_FAULT
} USBMH_USBState;

/* Static variables and handles */
static volatile USBMH_USBState state;
static unsigned char            memPoolHCD[HCDMEMORYPOOLSIZE];
static unsigned char            memPoolM[MMEMORYPOOLSIZE];
static volatile char            xPosition;
static volatile char            yPosition;
static volatile unsigned char   mouseButtons;
static USBMHType                mouseInstance;
static MutexP_Handle         mutexUSBWait;
static MutexP_Handle         mutexUSBLibAccess;
static SemaphoreP_Handle         semUSBConnected;

/* Function prototypes */
static void USBMH_hwiHandler(uintptr_t arg0);
void *serviceUSBHost(void *arg0);

/*
 *  ======== cbMouseHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the mouse.
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
static USBMHHandleType cbMouseHandler(USBMHType instance, USBMHEventType event,
		                              USBMHEventType eventMsg, void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            /* Set the mouse state for initialization */
            state = USBMH_INIT;
            SemaphoreP_post(semUSBConnected);
            break;

        case USB_EVENT_DISCONNECTED:
            /* Set the mouse state as not connected */
            state = USBMH_NO_DEVICE;
            break;

        case USBH_EVENT_HID_MS_PRESS:
            /* Set the button states */
            mouseButtons |= eventMsg;
            break;

        case USBH_EVENT_HID_MS_REL:
            /* Clear the button states */
            mouseButtons &= ~eventMsg;
            break;

        case USBH_EVENT_HID_MS_X:
            /* Update the variable containing the X coordinate */
            xPosition += (unsigned char)eventMsg;
            break;

        case USBH_EVENT_HID_MS_Y:
            /* Update the variable containing the Y coordinate */
            yPosition += (unsigned char)eventMsg;
            break;

        default:
            break;
    }
}

/*
 *  ======== USBMH_hwiHandler ========
 *  This function calls the USB library's host interrupt handler.
 */
static void USBMH_hwiHandler(uintptr_t arg0)
{
    USB0_IRQHostHandler();
}

/*
 *  ======== serviceUSBHost ========
 *  Task to periodically service the USB Stack
 *
 *  USBHCDMain handles the USB Stack's state machine. For example it handles the
 *  enumeration process when a device connects.
 *  Future USB library improvement goal is to remove this polling requirement..
 */
void *serviceUSBHost (void *arg0)
{
    unsigned int key;
    uint32_t time = 10;

    while (true) {
        key = MutexP_lock(mutexUSBLibAccess);
        USBHCDMain();
        MutexP_unlock(mutexUSBLibAccess, key);

        /* Send data periodically */
        usleep(time);
    }
}

/*
 *  ======== USBHCDEvents ========
 *  Generic USB Host Class Driver event callback.
 *
 *  This callback is called to notify the application that a unknown device was
 *  connected. (e.g. It wasn't a mouse)
 */
void USBHCDEvents(void *cbData)
{
    tEventInfo *pEventInfo;

    /* Cast this pointer to its actual type. */
    pEventInfo = (tEventInfo *)cbData;

    switch (pEventInfo->ui32Event) {

        case USB_EVENT_UNKNOWN_CONNECTED:
            /* An unknown device was detected. */
            state = USBMH_UNKNOWN;
            break;

        case USB_EVENT_DISCONNECTED:
            /* Unknown device has been removed. */
            state = USBMH_NO_DEVICE;
            break;

        case USB_EVENT_POWER_FAULT:
            /* No power means no device is present. */
            state = USBMH_POWER_FAULT;
            break;

        default:
            break;
    }
}

/*
 *  ======== USBMH_getState ========
 */
void USBMH_getState(USBMH_State *mouseState)
{
    unsigned int key;

    switch (state) {
        case USBMH_NO_DEVICE:
            break;

        case USBMH_INIT:
            state = USBMH_CONNECTED;

            /* Acquire lock */
            key = MutexP_lock(mutexUSBLibAccess);

            /* Reset global variables */
            xPosition = 0x00;
            yPosition = 0x00;
            mouseButtons = 0x00;

            USBHMouseInit(mouseInstance);

            /* Release lock */
            MutexP_unlock(mutexUSBLibAccess, key);

        default:
            break;
    }

    /* Return current status; regardless of the state */
    key = HwiP_disable();
    mouseState->deltaX = xPosition;
    mouseState->deltaY = yPosition;
    mouseState->button1 = (mouseButtons & HID_MOUSE_BUTTON_1) ? true : false;
    mouseState->button2 = (mouseButtons & HID_MOUSE_BUTTON_2) ? true : false;
    mouseState->button3 = (mouseButtons & HID_MOUSE_BUTTON_3) ? true : false;
    HwiP_restore(key);
}

/*
 *  ======== USBMH_init ========
 */
void USBMH_init(bool usbInternal)
{
    HwiP_Handle hwi;
    pthread_t         thread;
    pthread_attr_t    attrs;
    struct sched_param  priParam;
    int                 retc;
    uint32_t            ui32ULPI;
    uint32_t ui32PLLRate;

    Display_Handle display;

    Display_init();

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);

    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }

    /* Initialize the USB stack for host mode. */
    USBStackModeSet(0, eUSBModeHost, NULL);

    /* Register host class drivers */
    USBHCDRegisterDrivers(0, usbHostClassDrivers, numHostClassDrivers);

    /* Open an instance of the mouse host driver */
    mouseInstance = USBHMouseOpen(cbMouseHandler, memPoolM, MMEMORYPOOLSIZE);
    if(!mouseInstance) {
        Display_printf(display, 0, 0, "Error initializing the Mouse Host.\n");
        while(1);
    }

    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBMH_hwiHandler, NULL);
    if (hwi == NULL) {
        Display_printf(display, 0, 0, "Can't create USB Hwi.\n");
        while(1);
    }

    /* Check if the ULPI mode is to be used or not */
    if(!(usbInternal)) {
        ui32ULPI = USBLIB_FEATURE_ULPI_HS;
        USBHCDFeatureSet(0, USBLIB_FEATURE_USBULPI, &ui32ULPI);
    } else {
		/* Initialize USB power configuration */
		USBHCDPowerConfigInit(0, USBHCD_VBUS_AUTO_HIGH | USBHCD_VBUS_FILTER);
	}

     /*Gets the VCO frequency of 120MHZ */
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &ui32PLLRate);

    /*Set the PLL*/
    USBHCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &ui32PLLRate);

    /* Enable the USB stack */
    USBHCDInit(0, memPoolHCD, HCDMEMORYPOOLSIZE);

    /* RTOS primitives */
    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        Display_printf(display, 0, 0, "Could not create USB Connect semaphore.\n");
        while(1);
    }

    mutexUSBLibAccess = MutexP_create(NULL);
    if (mutexUSBLibAccess == NULL) {
        Display_printf(display, 0, 0, "Could not create USB mutex.\n");
        while(1);
    }

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        Display_printf(display, 0, 0, "Could not create USB wait mutex.\n");
        while(1);
    }
    Display_close(display);
    /*
     * Note that serviceUSBHost() should not be run until the USB Stack has been
     * initialized!!
     */

    pthread_attr_init(&attrs);
    priParam.sched_priority = sched_get_priority_max(SCHED_FIFO);


    retc = pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    pthread_attr_setschedparam(&attrs, &priParam);

    retc |= pthread_attr_setstacksize(&attrs, 768);
    if (retc != 0) {
           /* pthread_attr_setstacksize() failed */
           while (1);
    }
    retc = pthread_create(&thread, &attrs, serviceUSBHost, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }
}

/*
 *  ======== USBMH_waitForConnect ========
 */
bool USBMH_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);

    if (state == USBMH_NO_DEVICE) {
        if (!(SemaphoreP_pend(semUSBConnected, timeout)) == SemaphoreP_OK) {
            ret = false;
        }
    }

    MutexP_unlock(mutexUSBWait, key);

    return (ret);
}
