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
 *  ======== USBCDCD.c ========
 */

#include <stdbool.h>

/* Header files */
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
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdcdc.h>

/* Example/Board Header files */
#include "USBCDCD.h"
#include "ti_usblib_config.h"

typedef uint32_t            USBCDCDEventType;

/* Defines */

/* Typedefs */
typedef volatile enum {
    USBCDCD_STATE_IDLE = 0,
    USBCDCD_STATE_INIT,
    USBCDCD_STATE_UNCONFIGURED
} USBCDCD_USBState;

/* Static variables and handles */
static volatile USBCDCD_USBState state;

static MutexP_Handle mutexTxSerial;
static MutexP_Handle mutexRxSerial;
static MutexP_Handle mutexUSBWait;
static SemaphoreP_Handle semTxSerial;
static SemaphoreP_Handle semRxSerial;
static SemaphoreP_Handle semUSBConnected;

/* Function prototypes */
static void USBCDCD_hwiHandler(uintptr_t arg0);
static unsigned int rxData(unsigned char *pStr,
                           unsigned int length,
                           unsigned int timeout);
static unsigned int txData(char *pStr,
                           int length, unsigned int timeout);

static tLineCoding g_sLineCoding = {
    115200,                     /* 115200 baud rate. */
    1,                          /* 1 Stop Bit. */
    0,                          /* No Parity. */
    8                           /* 8 Bits of data. */
};

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
USBCDCDEventType cbRxHandler(void *cbData, USBCDCDEventType event,
                                    USBCDCDEventType eventMsg,
                                    void *eventMsgPtr)
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
USBCDCDEventType cbSerialHandler(void *cbData, USBCDCDEventType event,
                                        USBCDCDEventType eventMsg,
                                        void *eventMsgPtr)
{
    tLineCoding *psLineCoding;

    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            state = USBCDCD_STATE_INIT;
            SemaphoreP_post(semUSBConnected);
            break;

        case USB_EVENT_DISCONNECTED:
            state = USBCDCD_STATE_UNCONFIGURED;
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
USBCDCDEventType cbTxHandler(void *cbData, USBCDCDEventType event,
                                    USBCDCDEventType eventMsg,
                                    void *eventMsgPtr)
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
 *  ======== USBCDCD_hwiHandler ========
 *  This function calls the USB library's device interrupt handler.
 */
static void USBCDCD_hwiHandler(uintptr_t arg0)
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

    if (USBBufferDataAvailable(&g_sRxBuffer_serialDevice) || !(SemaphoreP_pend(semRxSerial, timeout) == SemaphoreP_TIMEOUT)) {
       read = USBBufferRead(&g_sRxBuffer_serialDevice, pStr, length);
    }

    return (read);
}


/*
 *  ======== txData ========
 */
static unsigned int txData(char *pStr,
                           int length, unsigned int timeout)
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
 *  ======== USBCDCD_init ========
 */
void USBCDCD_init(bool usbInternal)
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
    hwi = HwiP_create(INT_USB0, USBCDCD_hwiHandler, NULL);
    if (hwi == NULL) {
        Display_printf(display, 0, 0, "Can't create USB Hwi.\n");
        while(1);
    }

    /* RTOS primitives */
    semTxSerial = SemaphoreP_createBinary(0);
    if (semTxSerial == NULL) {
        Display_printf(display, 0, 0, "Can't create TX semaphore.\n");
        while(1);
    }

    semRxSerial = SemaphoreP_createBinary(0);
    if (semRxSerial == NULL) {
        Display_printf(display, 0, 0, "Can't create RX semaphore.\n");
        while(1);
    }

    semUSBConnected = SemaphoreP_createBinary(0);
    if (semUSBConnected == NULL) {
        Display_printf(display, 0, 0, "Can't create USB semaphore.\n");
        while(1);
    }

    mutexTxSerial = MutexP_create(NULL);
    if (mutexTxSerial == NULL) {
        Display_printf(display, 0, 0, "Can't create TX mutex.\n");
        while(1);
    }

    mutexRxSerial = MutexP_create(NULL);
    if (mutexRxSerial == NULL) {
        Display_printf(display, 0, 0, "Can't create RX mutex.\n");
        while(1);
    }

    mutexUSBWait = MutexP_create(NULL);
    if (mutexUSBWait == NULL) {
        Display_printf(display, 0, 0, "Could not create USB Wait mutex.\n");
        while(1);
    }

    /* State specific variables */
    state = USBCDCD_STATE_UNCONFIGURED;

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
    if (!USBDCDCInit(0, &serialDevice)) {
        Display_printf(display, 0, 0, "Error initializing the serial device.\n");
        while(1);
    }
    Display_close(display);
}

/*
 *  ======== USBCDCD_receiveData ========
 */
unsigned int USBCDCD_receiveData(unsigned char *pStr,
                                 unsigned int length,
                                 unsigned int timeout)
{
    unsigned int retValue = 0;
    unsigned int key;

    switch (state) {
        case USBCDCD_STATE_UNCONFIGURED:
            USBCDCD_waitForConnect(timeout);
            break;

        case USBCDCD_STATE_INIT:
            /* Acquire lock */
            key = MutexP_lock(mutexRxSerial);

            USBBufferInit(&g_sTxBuffer_serialDevice);
            USBBufferInit(&g_sRxBuffer_serialDevice);

            state = USBCDCD_STATE_IDLE;

            retValue = rxData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexRxSerial, key);
            break;

        case USBCDCD_STATE_IDLE:
            /* Acquire lock */
            key = MutexP_lock(mutexRxSerial);

            retValue = rxData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexRxSerial, key);
            break;

        default:
            break;
    }

    return (retValue);
}

/*
 *  ======== USBCDCD_sendData ========
 */
unsigned int USBCDCD_sendData(char *pStr,
                              unsigned int length,
                              unsigned int timeout)
{
    unsigned int retValue = 0;
    unsigned int key;

    switch (state) {
        case USBCDCD_STATE_UNCONFIGURED:
            USBCDCD_waitForConnect(timeout);
            break;

        case USBCDCD_STATE_INIT:
            /* Acquire lock */
            key = MutexP_lock(mutexTxSerial);

            USBBufferInit(&g_sTxBuffer_serialDevice);
            USBBufferInit(&g_sRxBuffer_serialDevice);

            state = USBCDCD_STATE_IDLE;

            retValue = txData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexTxSerial, key);
            break;

        case USBCDCD_STATE_IDLE:
            /* Acquire lock */
            key = MutexP_lock(mutexTxSerial);

            retValue = txData(pStr, length, timeout);

            /* Release lock */
            MutexP_unlock(mutexTxSerial, key);
            break;

        default:
            break;
    }

    return (retValue);
}

/*
 *  ======== USBCDCD_waitForConnect ========
 */
bool USBCDCD_waitForConnect(unsigned int timeout)
{
    bool ret = true;
    unsigned int key;

    /* Need exclusive access to prevent a race condition */
    key = MutexP_lock(mutexUSBWait);

    if (state == USBCDCD_STATE_UNCONFIGURED) {
        if (SemaphoreP_pend(semUSBConnected, timeout)== SemaphoreP_TIMEOUT) {
            ret = false;
        }
    }

    MutexP_unlock(mutexUSBWait, key);

    return (ret);
}
