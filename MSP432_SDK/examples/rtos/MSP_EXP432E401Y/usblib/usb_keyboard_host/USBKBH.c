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
 *  ======== USBKBH.c ========
 */
/*Header files */
#include <stdbool.h>
/* For usleep() */
#include <unistd.h>
#include <ti/display/Display.h>

/* POSIX Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/MutexP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/host/usbhost.h>
#include <ti/usblib/msp432e4/host/usbhhid.h>
#include <ti/usblib/msp432e4/host/usbhhidkeyboard.h>

/* POSIX Header files */
#include <pthread.h>

/* Example/Board Header files */
#include "USBKBH.h"
#include "ti_usblib_config.h"

typedef tUSBHKeyboard          *USBKBHType;
typedef uint32_t                USBKBHEventType;
typedef void                    USBKBHHandleType;

/* Defines */
#define HCDMEMORYPOOLSIZE   128 /* Memory for the Host Class Driver */
#define KBMEMORYPOOLSIZE    128 /* Memory for the keyboard host driver */

/* Typedefs */
typedef volatile enum {
    USBKBH_NO_DEVICE = 0,
    USBKBH_INIT,
    USBKBH_CONNECTED,
    USBKBH_UNKNOWN,
    USBKBH_POWER_FAULT
} USBKBH_USBState;

/* Static variables and handles */
static volatile USBKBH_USBState state;
static unsigned char    memPoolHCD[HCDMEMORYPOOLSIZE];
static unsigned char    memPoolKB[KBMEMORYPOOLSIZE];
static volatile int     kbCh;
static volatile unsigned char   kbLEDs;
static USBKBHType       keyboardInstance;

static MutexP_Handle mutexUSBLibAccess;
static MutexP_Handle mutexUSBWait;

static SemaphoreP_Handle semKeyboard;
static SemaphoreP_Handle semUSBConnected;

/* External Host keyboard map provided with the USB library */
extern const tHIDKeyboardUsageTable g_sUSKeyboardMap;

/* Function prototypes */
static int getC(unsigned int time);
static void USBKBH_hwiHandler(uintptr_t arg0);
void *serviceUSBHost(void *arg0);

/*
 *  ======== cbKeyboardHandler ========
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

USBKBHHandleType cbKeyboardHandler(USBKBHType instance, USBKBHEventType event,
                                          USBKBHEventType eventMsg, void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            /* Set the keyboard state for initialization */
            state = USBKBH_INIT;
            SemaphoreP_post(semUSBConnected);
            break;

        case USB_EVENT_DISCONNECTED:
            /* Set the keyboard state as not connected */
            state = USBKBH_NO_DEVICE;
            break;

        case USBH_EVENT_HID_KB_PRESS:
            /* A key was pressed, determine which one */
            switch (eventMsg) {
                case HID_KEYB_USAGE_CAPSLOCK:
                    /* Toggle CAPLOCK LED */
                    kbLEDs ^= HID_KEYB_CAPS_LOCK;
                    break;

                /* SCROLL LOCK is not defined in the USB library */
                case 0x47:
                    kbLEDs ^= HID_KEYB_SCROLL_LOCK;
                    break;

                /* NUM Lock is not defined in the USB library */
                case 0x53:
                    kbLEDs ^= HID_KEYB_NUM_LOCK;
                    break;

                case HID_KEYB_USAGE_BACKSPACE:
                    kbCh = '\b';
                    SemaphoreP_post(semKeyboard);
                    break;

                /* Else, it's a character */
                default:
                    kbCh = USBHKeyboardUsageToChar(
                            keyboardInstance,
                            &g_sUSKeyboardMap,
                            (unsigned char) eventMsg);

                    SemaphoreP_post(semKeyboard);
                    break;
            }
            break;

        case USBH_EVENT_HID_KB_MOD:
            break;

        case USBH_EVENT_HID_KB_REL:
            break;

        default:
            break;
    }
}

/*
 *  ======== getC ========
 */
static int getC(unsigned int time)
{
    /* Wait for the callback handler to post the semaphore */
    if (SemaphoreP_pend(semKeyboard, time) == SemaphoreP_TIMEOUT) {
        return (0);
    }
    else {
        return (kbCh);
    }
}

/*
 *  ======== USBKBH_hwiHandler ========
 *  This function calls the USB library's host interrupt handler.
 */
static void USBKBH_hwiHandler(uintptr_t arg0)
{
    USB0_IRQHostHandler();
}

/*
 *  ======== serviceUSBHost ========
 *  Task to periodically service the USB Stack
 *
 *  USBHCDMain handles the USB Stack's state machine. For example it handles
 *  the enumeration process when a device connects.
 *  Future USB library improvement goal is to remove this polling requirement.
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
 *  connected. (e.g. It wasn't a keyboard)
 */
void USBHCDEvents(void *cbData)
{
    tEventInfo *pEventInfo;

    /* Cast this pointer to its actual type. */
    pEventInfo = (tEventInfo *)cbData;

    switch (pEventInfo->ui32Event) {

        case USB_EVENT_UNKNOWN_CONNECTED:
            /* An unknown device was detected. */
            state = USBKBH_UNKNOWN;
            break;

        case USB_EVENT_DISCONNECTED:
            /* Unknown device has been removed. */
            state = USBKBH_NO_DEVICE;
            break;

        case USB_EVENT_POWER_FAULT:
            /* No power means no device is present. */
            state = USBKBH_POWER_FAULT;
            break;

        default:
            break;
    }
}

/*
 *  ======== USBKBH_getChar ========
 */
int USBKBH_getChar(unsigned int timeout)
{
    unsigned int key;
    int ch = 0;

    switch (state) {
        case USBKBH_NO_DEVICE:
            USBKBH_waitForConnect(timeout);
            break;

        case USBKBH_INIT:
            key = MutexP_lock(mutexUSBLibAccess);
            USBHKeyboardInit(keyboardInstance);
            MutexP_unlock(mutexUSBLibAccess, key);
            ch = getC(timeout);

            state = USBKBH_CONNECTED;

            break;

        case USBKBH_CONNECTED:
            ch = getC(timeout);
            break;

        default:
            break;
    }

    return (ch);
}

/*
 *  ======== USBKBH_getState ========
 */
void USBKBH_getState(USBKBH_State *keyboardState)
{
    unsigned int key;

    key = HwiP_disable();
    keyboardState->numLED = (kbLEDs & HID_KEYB_NUM_LOCK) ? true : false;
    keyboardState->capsLED = (kbLEDs & HID_KEYB_CAPS_LOCK) ? true : false;
    keyboardState->scrollLED = (kbLEDs & HID_KEYB_SCROLL_LOCK) ? true : false;
    HwiP_restore(key);
}

/*
 *  ======== USBKBH_getString ========
 */
int USBKBH_getString(char *chArray, unsigned int length, unsigned int timeout)
{
    int ch;
    int count = 0;

    if (!length) {
        return (0);
    }

    do {
        ch = USBKBH_getChar(timeout);

        if (ch == '\n') {
            ch = 0;
        }
        else if (ch == '\b') {
            if (count) {
                count--;
            }
            continue;
        }
        chArray[count] = ch;
        count++;

    } while ((ch != 0) && ((count - 1) < length));

    return (count - 1);
}

/*
 *  ======== USBKBH_init ========
 */
void USBKBH_init(bool usbInternal)
{
    HwiP_Handle hwi;
    uint32_t ui32PLLRate;

    pthread_t         thread;
    pthread_attr_t    attrs;
    struct sched_param  priParam;
    int                 retc;
    uint32_t            ui32ULPI;

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

    /* Open an instance of the keyboard host driver */
    keyboardInstance = USBHKeyboardOpen(cbKeyboardHandler, memPoolKB, KBMEMORYPOOLSIZE);
    if(!keyboardInstance) {
        //Error initializing the Keyboard Host
        Display_printf(display, 0, 0, "Error initializing the Keyboard Host.\n");
        while(1);
    }


    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBKBH_hwiHandler, NULL);
    if (hwi == NULL) {
        //Can't create USB Hwi
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
    semKeyboard = SemaphoreP_createBinary(0);
    if (semKeyboard == NULL) {
        //Can't create keyboard semaphore
        Display_printf(display, 0, 0, "Can't create keyboard semaphore.\n");
        while(1);

    }

    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        //Can't create USB semaphore
        Display_printf(display, 0, 0, "Can't create USB semaphore.\n");
        while(1);
    }

    mutexUSBLibAccess = MutexP_create(NULL);
    if (mutexUSBLibAccess == NULL) {
        //Can't create USB mutex
        Display_printf(display, 0, 0, "Can't create USB mutex.\n");
        while(1);
    }

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        //Can't create USB wait mutex
        Display_printf(display, 0, 0, "Can't create USB wait mutex.\n");
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

    /* State specific variables */
    kbLEDs = 0x00;
    kbCh = 0x00;

}

/*
 *  ======== USBKBH_setState ========
 */
void USBKBH_setState(USBKBH_State *keyboardState)
{
    unsigned int key;
    unsigned char leds = 0;

    if (state == USBKBH_CONNECTED) {
        /* Set the bit packed LED value */
        leds |= (keyboardState->numLED) ? HID_KEYB_NUM_LOCK : 0;
        leds |= (keyboardState->capsLED) ? HID_KEYB_CAPS_LOCK : 0;
        leds |= (keyboardState->scrollLED) ? HID_KEYB_SCROLL_LOCK : 0;

        key = MutexP_lock(mutexUSBLibAccess);
        USBHKeyboardModifierSet(keyboardInstance, leds);
        MutexP_unlock(mutexUSBLibAccess, key);
    }
}

/*
 *  ======== USBKBH_waitForConnect ========
 */
bool USBKBH_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);

    if (state == USBKBH_NO_DEVICE) {
        if (SemaphoreP_pend(semUSBConnected, timeout) == SemaphoreP_TIMEOUT) {
            ret = false;
        }
    }

    MutexP_unlock(mutexUSBWait, key);

    return (ret);
}
