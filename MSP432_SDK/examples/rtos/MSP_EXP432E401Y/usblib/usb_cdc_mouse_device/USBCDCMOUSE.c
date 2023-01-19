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
 *    ======== USBCDCMOUSE.c ========
 */

/* XDCtools Header files */
#include <ti/display/Display.h>

/* POSIX Header files */
#include <ti/drivers/dpl/SemaphoreP.h>

#include <stdbool.h>

/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcomp.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>
#include <ti/usblib/msp432e4/device/usbdhid.h>
#include <ti/usblib/msp432e4/device/usbdhidmouse.h>

/* POSIX Header files */
#include <pthread.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/MutexP.h>


/* Example/Board Header files */
#include "USBCDCMOUSE.h"
#include "ti_usblib_config.h"

typedef uint32_t            USBCDCMOUSEEventType;


/* Defines */

/* Typedefs */
typedef volatile enum {
    USBCDCMOUSE_CONNECTED = 0,
    USBCDCMOUSE_DISCONNECTED
} USBCDCMOUSE_USBState;

typedef volatile enum {
    USBCDCMOUSE_CDC_IDLE = 0,
    USBCDCMOUSE_CDC_INIT,
    USBCDCMOUSE_CDC_UNCONFIGURED,
} USBCDCMOUSE_USBCDCState;

typedef volatile enum {
    USBCDCMOUSE_M_IDLE = 0,
    USBCDCMOUSE_M_SENDING,
    USBCDCMOUSE_M_UNCONFIGURED
} USBCDCMOUSE_USBMState;

/* Static variables and handles */
static USBCDCMOUSE_USBCDCState  stateCDC;
static USBCDCMOUSE_USBMState    stateM;
static USBCDCMOUSE_USBState     stateUSB;
static uint8_t                  descriptorData[DESCRIPTOR_DATA_SIZE_compositeDevice];


static MutexP_Handle mutexTxSerial;
static MutexP_Handle mutexRxSerial;
static MutexP_Handle mutexMouse;
static MutexP_Handle mutexUSBWait;

static SemaphoreP_Handle semTxSerial;
static SemaphoreP_Handle semRxSerial;
static SemaphoreP_Handle semMouse;
static SemaphoreP_Handle semUSBConnected;


/* Function prototypes */
static void  USBCDCMOUSE_hwiHandler(uintptr_t arg0);
static unsigned int  rxData(unsigned char *pStr,
                            unsigned int length,
                            unsigned int timeout);
static int           sendState(USBCDCMOUSE_State *mouseState,
                               unsigned int timeout);
static unsigned int  txData(char *pStr,
                            int length,
                            unsigned int timeout);
static bool          waitUntilSent(unsigned int timeout);

extern void USB0_IRQDeviceHandler(void);

static tLineCoding g_sLineCoding = {
    115200,                     /* 115200 baud rate. */
    1,                          /* 1 Stop Bit. */
    0,                          /* No Parity. */
    8                           /* 8 Bits of data. */
};

/*
 *  ======== cbCompositeHandler ========
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
USBCDCMOUSEEventType cbCompositeHandler(void *cbData, USBCDCMOUSEEventType event,
                                USBCDCMOUSEEventType eventMsg, void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            if (stateUSB != USBCDCMOUSE_CONNECTED) {
                stateUSB = USBCDCMOUSE_CONNECTED;

                SemaphoreP_post(semUSBConnected);
            }
            break;

        case USB_EVENT_DISCONNECTED:
            stateUSB = USBCDCMOUSE_DISCONNECTED;
            break;

        default:
            break;
    }

    return (0);
}

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
USBCDCMOUSEEventType cbMouseHandler(void *cbData, USBCDCMOUSEEventType event,
                            USBCDCMOUSEEventType eventMsg, void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            /* Handled by the cbCompositeHandler callback */
            stateM = USBCDCMOUSE_M_IDLE;
            break;

        case USB_EVENT_DISCONNECTED:
            if (stateM == USBCDCMOUSE_M_SENDING) {
                stateM = USBCDCMOUSE_M_UNCONFIGURED;

                SemaphoreP_post(semMouse);
            }
            else {
                stateM = USBCDCMOUSE_M_UNCONFIGURED;
            }
            break;

        case USB_EVENT_TX_COMPLETE:
            stateM = USBCDCMOUSE_M_IDLE;

            SemaphoreP_post(semMouse);
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbRxHandler ========
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
USBCDCMOUSEEventType cbRxHandler(void *cbData, USBCDCMOUSEEventType event,
                         USBCDCMOUSEEventType eventMsg, void *eventMsgPtr)
{
    switch (event) {
        case USB_EVENT_RX_AVAILABLE:

            SemaphoreP_post(semRxSerial);
            break;

        case USB_EVENT_DATA_REMAINING:
            break;

        case USB_EVENT_REQUEST_BUFFER:
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbSerialHandler ========
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
USBCDCMOUSEEventType cbSerialHandler(void *cbData, USBCDCMOUSEEventType event,
                             USBCDCMOUSEEventType eventMsg, void *eventMsgPtr)
{
    tLineCoding *psLineCoding;

    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            stateCDC = USBCDCMOUSE_CDC_INIT;
            break;

        case USB_EVENT_DISCONNECTED:
            stateCDC = USBCDCMOUSE_CDC_UNCONFIGURED;
            break;

        case USBD_CDC_EVENT_GET_LINE_CODING:
            /* Create a pointer to the line coding information. */
            psLineCoding = (tLineCoding *)eventMsgPtr;

            /* Copy the current line coding information into the structure. */
            *(psLineCoding) = g_sLineCoding;
            break;

        case USBD_CDC_EVENT_SET_LINE_CODING:
            /* Create a pointer to the line coding information. */
            psLineCoding = (tLineCoding *)eventMsgPtr;

            /*
             * Copy the line coding information into the current line coding
             * structure.
             */
            g_sLineCoding = *(psLineCoding);
            break;

        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            break;

        case USBD_CDC_EVENT_SEND_BREAK:
            break;

        case USBD_CDC_EVENT_CLEAR_BREAK:
            break;

        case USB_EVENT_SUSPEND:
            break;

        case USB_EVENT_RESUME:
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbTxHandler ========
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
USBCDCMOUSEEventType cbTxHandler(void *cbData, USBCDCMOUSEEventType event,
                         USBCDCMOUSEEventType eventMsg, void *eventMsgPtr)
{
    switch (event) {
        case USB_EVENT_TX_COMPLETE:
            /*
             * Data was sent, so there should be some space available on the
             * buffer
             */
            SemaphoreP_post(semTxSerial);
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== USBCDCMOUSE_hwiHandler ========
 *  This function calls the USB library's device interrupt handler.
 */
static void USBCDCMOUSE_hwiHandler(uintptr_t arg0)
{
    USB0_IRQDeviceHandler();
}

/*
 *  ======== rxData ========
 */
static unsigned int rxData(unsigned char *pStr,
                           unsigned int length,
                           unsigned int timeout)
{
    unsigned int read = 0;

    if (USBBufferDataAvailable(&g_sRxBuffer_serialDevice) || (SemaphoreP_pend(semRxSerial, timeout) == SemaphoreP_OK)) {
            read = USBBufferRead(&g_sRxBuffer_serialDevice, pStr, length);
        }

    return (read);
}

/*
 *  ======== sendState ========
 */
static int sendState(USBCDCMOUSE_State *mouseState, unsigned int timeout)
{
    unsigned int key;
    int retValue = 0;
    unsigned char buttons = 0;

    /* Set the bit packed button values */
    buttons |= (mouseState->button1) ? HID_MOUSE_BUTTON_1 : 0;
    buttons |= (mouseState->button2) ? HID_MOUSE_BUTTON_2 : 0;
    buttons |= (mouseState->button3) ? HID_MOUSE_BUTTON_3 : 0;

    key = MutexP_lock(mutexMouse);

    stateM = USBCDCMOUSE_M_SENDING;
    retValue = USBDHIDMouseStateChange((tUSBDHIDMouseDevice *)&mouseDevice,
                                        mouseState->deltaX,
                                        mouseState->deltaY,
                                        buttons);

    if (!retValue) {
        retValue = !waitUntilSent(timeout);
    }

    MutexP_unlock(mutexMouse, key);

    return (retValue);
}

/*
 *  ======== txData ========
 */
static unsigned int txData(char *pStr,
                           int length,
                           unsigned int timeout)
{
    unsigned int buffAvailSize;
    unsigned int bufferedCount = 0;
    unsigned int sendCount = 0;
    unsigned char *sendPtr;

    while (bufferedCount != length) {
        /* Determine the buffer size available */
        buffAvailSize = USBBufferSpaceAvailable(&g_sTxBuffer_serialDevice);

        /* Determine how much needs to be sent */
        if ((length - bufferedCount) > buffAvailSize) {
            sendCount = buffAvailSize;
        }
        else {
            sendCount = length - bufferedCount;
        }

        /* Adjust the pointer to the data */
        sendPtr = (unsigned char *)pStr + bufferedCount;

        /* Place the contents into the USB BUffer */
        bufferedCount += USBBufferWrite(&g_sTxBuffer_serialDevice, sendPtr, sendCount);

        /* Pend until some data was sent through the USB*/
        if (SemaphoreP_pend(semTxSerial, timeout) == SemaphoreP_TIMEOUT) {
            break;
        }
    }

    return (bufferedCount);
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

    if (SemaphoreP_pend(semMouse, timeout)  == SemaphoreP_TIMEOUT) {
            stateM = USBCDCMOUSE_M_UNCONFIGURED;
            return (false);
        }


    return (true);
}

/*
 *  ======== USBCDCMOUSE_init ========
 */

void USBCDCMOUSE_init(bool usbInternal)
{
    HwiP_Handle hwi;
    uint32_t ui32ULPI;
    uint32_t ui32PLLRate;

    Display_Handle display;

    Display_init();

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);

    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }

    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBCDCMOUSE_hwiHandler, NULL);
    if (hwi == NULL) {
        //Can't create USB Hwi
        Display_printf(display, 0, 0, "Failed to create USB Hwi.\n");
        while(1);

    }

    /* RTOS primitives */
    semTxSerial = SemaphoreP_createBinary(0);
    if (semTxSerial == NULL) {
        //Can't create TX semaphore
        Display_printf(display, 0, 0, "Failed to create TX semaphore.\n");
        while(1);
    }

    semRxSerial = SemaphoreP_createBinary(0);
    if (semRxSerial == NULL) {
        //Can't create RX semaphore
        Display_printf(display, 0, 0, "Failed to create RX semaphore.\n");
        while(1);
    }

    semMouse = SemaphoreP_createBinary(0);
    if (semMouse == NULL) {
        //Can't create mouse semaphore
        Display_printf(display, 0, 0, "Failed to create mouse semaphore.\n");
        while(1);
    }

    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        //Can't create USB semaphore
        Display_printf(display, 0, 0, "Failed to create USB semaphore.\n");
        while(1);
    }

    mutexTxSerial = MutexP_create(NULL);
    if (mutexTxSerial == NULL) {
        //Can't create TX mutex
        Display_printf(display, 0, 0, "Failed to create TX mutex.\n");
        while(1);
    }

    mutexRxSerial = MutexP_create(NULL);
    if (mutexRxSerial == NULL) {
        //Can't create RX mutex
        Display_printf(display, 0, 0, "Failed to create RX mutex.\n");
        while(1);
    }

    mutexMouse = MutexP_create(NULL);
    if (mutexMouse == NULL) {
        //Can't create mouse mutex
        Display_printf(display, 0, 0, "Failed to create mouse mutex.\n");
        while(1);
    }

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        //Can't create USB wait mutex
        Display_printf(display, 0, 0, "Failed to create USB wait mutex.\n");
        while(1);
    }

    /* State specific variables */
    stateUSB = USBCDCMOUSE_DISCONNECTED;
    stateCDC = USBCDCMOUSE_CDC_UNCONFIGURED;
    stateM   = USBCDCMOUSE_M_UNCONFIGURED;

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
    /* Install the composite instances */
    if (!USBDHIDMouseCompositeInit(0, &mouseDevice, &compositeDevice_entries[0])) {
        //Can't initialize HID mouse composite component
        Display_printf(display, 0, 0, "Can't initialize HID Mouse composite component.\n");
        while(1);
    }

    if (!USBDCDCCompositeInit(0, &serialDevice, &compositeDevice_entries[1])) {
        //Can't initialize CDC composite component
        Display_printf(display, 0, 0, "Can't initialize CDC composite component.\n");
        while(1);
    }


    if (!compositeDevice_entries[0].pvInstance || !compositeDevice_entries[1].pvInstance){
        //Can't initialize Error initializing the composite device
        Display_printf(display, 0, 0, "Error initializing the composite device.\n");
        while(1);
    }

    Display_close(display);

    /* Initialize the USB stack with the composite device */
    USBDCompositeInit(0, (tUSBDCompositeDevice *) &compositeDevice,
                      DESCRIPTOR_DATA_SIZE_compositeDevice, descriptorData);
}

/*
 *  ======== USBCDCMOUSE_receiveData ========
 */
unsigned int USBCDCMOUSE_receiveData(unsigned char *pStr,
                                     unsigned int length,
                                     unsigned int timeout)
{
    unsigned int retValue = 0;
    unsigned int key;

    switch (stateCDC) {
        case USBCDCMOUSE_CDC_UNCONFIGURED:
            break;

        case USBCDCMOUSE_CDC_INIT:

            key = MutexP_lock(mutexRxSerial);

            USBBufferInit(&g_sTxBuffer_serialDevice);
            USBBufferInit(&g_sRxBuffer_serialDevice);
            stateCDC = USBCDCMOUSE_CDC_IDLE;
            retValue = rxData(pStr, length, timeout);

            MutexP_unlock(mutexRxSerial, key);
            break;

        case USBCDCMOUSE_CDC_IDLE:

            key = MutexP_lock(mutexRxSerial);
            retValue = rxData(pStr, length, timeout);
            MutexP_unlock(mutexRxSerial, key);
            break;

        default:
            break;
    }

    return (retValue);
}

/*
 *  ======== USBCDCMOUSE_sendData ========
 */
unsigned int USBCDCMOUSE_sendData(char *pStr,
                                  unsigned int length,
                                  unsigned int timeout)
{
    unsigned int retValue = 0;
    unsigned int key;

    switch (stateCDC) {
        case USBCDCMOUSE_CDC_UNCONFIGURED:
            break;

        case USBCDCMOUSE_CDC_INIT:

            key = MutexP_lock(mutexTxSerial);

            USBBufferInit(&g_sTxBuffer_serialDevice);
            USBBufferInit(&g_sRxBuffer_serialDevice);
            stateCDC = USBCDCMOUSE_CDC_IDLE;
            retValue = txData(pStr, length, timeout);

            MutexP_unlock(mutexTxSerial, key);

            break;

        case USBCDCMOUSE_CDC_IDLE:

            key = MutexP_lock(mutexTxSerial);

            retValue = txData(pStr, length, timeout);

            MutexP_unlock(mutexTxSerial, key);
            break;

        default:
            break;
    }

    return (retValue);
}

/*
 *  ======== USBCDCMOUSE_setState ========
 */
unsigned int USBCDCMOUSE_setState(USBCDCMOUSE_State *mouseState,
                                  unsigned int timeout)
{
    static unsigned int retValue = 0;

    if (stateM == USBCDCMOUSE_M_IDLE) {
        retValue = sendState(mouseState, timeout);
    }

    return (retValue);
}

/*
 *  ======== USBCDCMOUSE_waitForConnect ========
 */
bool USBCDCMOUSE_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);


    if (stateUSB != USBCDCMOUSE_CONNECTED) {
        if (SemaphoreP_pend(semUSBConnected, timeout) == SemaphoreP_TIMEOUT) {
            ret = false;
        }
    }
    MutexP_unlock(mutexUSBWait, key);


    return (ret);
}
