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
 *  ======== usbcdcserialhost.c ========
 */

/*Stancdard C Header files */
#include <stdio.h>
#include <stdbool.h>

/*TI driver header files*/
#include <ti/drivers/UART.h>
#include <ti/drivers/dpl/HwiP.h>

/* POSIX Header file */
#include <pthread.h>

/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbcdc.h>
#include <ti/usblib/msp432e4/host/usbhost.h>
#include <ti/usblib/msp432e4/host/usbhcdc.h>
#include <ti/usblib/msp432e4/host/usbhcdcserial.h>
#include <usbcdcserialhost.h>

#include "ti_drivers_config.h"
#include "ti_usblib_config.h"

/* Example Header file */
#include "usbcdcserialhost.h"

typedef tUSBHCDCSerial         *USBCDCHType;


/* Defines */

/* Memory for the Host Class Driver */
#define HCDMEMORYPOOLSIZE   128

/* The bulk enpoint interface number*/
#define INTERFACE_1      1

/* UART tx and rx array size*/
#define DATA_SIZE      64


/* Typedefs */
/* CDC states*/
typedef volatile enum {
    USBCDCH_NO_DEVICE = 0,
    USBCDCH_INIT,
    USBCDCH_CONNECTED,
    USBCDCH_UNKNOWN,
    USBCDCH_POWER_FAULT
} USBCDCH_USBState;


/* Static variables and handles */

/* Device states */
static volatile USBCDCH_USBState state;

/* Array for CDC descriptor data */
static unsigned char    memPoolHCD[HCDMEMORYPOOLSIZE];

/* character counter*/
uint32_t uartBufLen = 0;

/* UART transmit data array*/
char g_pcTXBuf[DATA_SIZE];

/* The instance data for the CDC driver.*/
tUSBHCDCSerial *g_psCDCSerialInstance = 0;

/* UART handle*/
UART_Handle uartHandle;

/* UART Parameter */
UART_Params uartParams;


/* Function prototypes */
static void cdcSerialHandler(USBCDCHType cdcInstance, uint32_t event,
                                          uint32_t eventMsg, void *eventMsgPtr);
static void USBCDCH_hwiHandler(uintptr_t arg0);

/*
 *  ======== cdcSerialHandler ========
 *  Callback handler.
 *
 *  Callback handler called by the USB stack to notify us on when
 *  data is received from the device.
 *
 */
static void cdcSerialHandler(USBCDCHType cdcInstance, uint32_t event,
                             uint32_t eventMsg, void *eventMsgPtr)
{
    unsigned char ucChar;
    uint8_t i;


    /* Determine what event has happened */
    switch (event)
    {

        /* Data sent by the device was detected.*/
        case USBH_EVENT_RX_CDC_DATA:
        {

            /* parse through 64 bytes of received data to display*/
            for (i = 0; i<DATA_SIZE; i++)
            {

                /* Retrieve the data value pointed to by i */
                ucChar = (unsigned char)USBHCDCProcessData(g_psCDCSerialInstance, i);

                /* Print data to UART terminal */
                uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "%c", ucChar);
                UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
            }

            break;
       }
        /* Data sent to device */
       case USBH_EVENT_TX_CDC_DATA:
       {

           /*
            *Nothing to process here since we don't care about the result of
            *sending the data to the device.
            */
           break;
       }

       default:
           break;
    }
}


/*
 *  ======== USBCDCH_hwiHandler ========
 *  This function calls the USB library's host interrupt handler.
 */
static void USBCDCH_hwiHandler(uintptr_t arg0)
{
    USB0_IRQHostHandler();
}



/*
 *  ======== USBHCDEvents ========
 *  Generic USB Host Class Driver event callback.
 *
 *  This callback is called to notify the application that a device was
 *  connected.
 */
void USBHCDEvents(void *eventData)
{
    tEventInfo *pEventInfo;

    /* Cast this pointer to its actual type. */
    pEventInfo = (tEventInfo *)eventData;

    switch (pEventInfo->ui32Event) {

        case USB_EVENT_CONNECTED:
             /*
              * See if this is a CDC device where control interface descriptor
              * (interface 0) is defined as CDC class and interface protocol is
              * Common AT commands (value of 02)
              */
             if((USBHCDDevClass(pEventInfo->ui32Instance, 0) == USB_CLASS_CDC) &&
                (USBHCDDevProtocol(pEventInfo->ui32Instance, 0) ==
                        USB_CDC_PROTOCOL_V25TER))
             {

                 /* Indicate that the CDC device has been detected.*/
                 uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "\nCDC Device Connected\n");
                 UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

                 /*
                  * Proceed to the USBCDCH_INIT state so that the main
                  * loop can finish initializing the CDC device since
                  * USBCDCSerialInit() cannot be called from within a callback.
                  */
                 state = USBCDCH_INIT;
             }

        break;

        case USB_EVENT_UNKNOWN_CONNECTED:

            /* An unknown device was detected. */
            uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "\nUnknown device connected\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
            state = USBCDCH_UNKNOWN;

            break;

        case USB_EVENT_DISCONNECTED:

            /* Indicate that the device has been disconnected.*/
            uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "\nCDC Device Disconnected\n");
            UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

            /* Unknown device has been removed. */
            state = USBCDCH_NO_DEVICE;
            break;

        case USB_EVENT_POWER_FAULT:
            /* No power means no device is present. */
            state = USBCDCH_POWER_FAULT;
            break;

        default:
            break;
    }
}


/*
 *  ======== USBCDCH_init ========
 */
void USBCDCH_init(bool usbInternal)
{
    HwiP_Handle hwi;

    uint32_t PLLRate;
    uint32_t ui32ULPI;

    /* Initialize the USB stack for host mode. */
    USBStackModeSet(0, eUSBModeHost, NULL);

    /* Register host class drivers */
    USBHCDRegisterDrivers(0, usbHostClassDrivers, numHostClassDrivers);

    /* Open an instance of the CDC host driver */
    g_psCDCSerialInstance = USBHCDCSerialOpen(cdcSerialHandler);
    if(!g_psCDCSerialInstance) {

        /*Error initializing the CDC Host*/
        uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "Error initializing the CDC Host.\n");
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

        while(1);
    }

    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBCDCH_hwiHandler, NULL);
    if (hwi == NULL) {
        /*Can't create USB Hwi*/
        uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "Can't create USB Hwi.\n");
        UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);
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
  
    /* Tell the USB library the CPU clock and the PLL frequency.*/
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &PLLRate);

    USBHCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &PLLRate);

    /* Enable the USB stack */
    USBHCDInit(0, memPoolHCD, HCDMEMORYPOOLSIZE);

    /* Print to UART terminal */

    /* clear the UART terminal*/
    uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "\033[2J\033[H");
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

    uartBufLen = snprintf(g_pcTXBuf, DATA_SIZE, "CDC Serial Application\n");
    UART_writePolling(uartHandle, g_pcTXBuf, uartBufLen);

}

/*
 *  ======== cdcSerialHostFxn ========
 *  Task for this example
 */
void *cdcSerialHostFxn (void *arg0)
{

    uint8_t dataBuffer[DATA_SIZE];
    uint8_t dataSize;
    uint8_t rxCharCount = 0, dataBufIdx=0;

    bool *usbInternal = (bool *) arg0;

    /*initialize UART*/
    UART_init();

    /*initialize UART prameters with default values*/
    UART_Params_init(&uartParams);

    /* Set UART echo off*/
    uartParams.readEcho = UART_ECHO_OFF;

    /* create UART handle */
    uartHandle = UART_open(CONFIG_UART_0, &uartParams);

    if (uartHandle == NULL)
    {
        while(1);
    }

    /* Initialize CDC host*/
    USBCDCH_init(usbInternal);

    while(1)
    {
         /* Call the host library*/
         USBHCDMain();

         switch (state)
         {
             case USBCDCH_INIT:
             {
                 /* Initialize CDC serial */
                 USBHCDCSerialInit(g_psCDCSerialInstance);

                 state = USBCDCH_CONNECTED;
             break;
             }

             case USBCDCH_CONNECTED:
             {
                 /* poll for data from device */
                 USBHCDCGetDataFromDevice(g_psCDCSerialInstance, INTERFACE_1);

                 dataSize = 1;

                 /* check for user entered UART data */
                 UART_control(uartHandle, UART_CMD_GETRXCOUNT, (void *)&rxCharCount);

                 /* User has pressed a key.  There is at least one character received from
                  * UART
                  */
                 if(rxCharCount > 0)
                 {
                      /* Get the user entered character */
                      UART_read(uartHandle, dataBuffer + dataBufIdx, rxCharCount);

                      /* Send the character to the device */
                      USBHCDCSendDataToDevice(g_psCDCSerialInstance, INTERFACE_1, dataBuffer, dataSize);

                 }

                 break;
             }
             case USBCDCH_UNKNOWN:
             {
                 break;
             }

             case USBCDCH_NO_DEVICE:
             {
                 break;
             }
             default:
             {
                 break;
             }
         }
    }
}
