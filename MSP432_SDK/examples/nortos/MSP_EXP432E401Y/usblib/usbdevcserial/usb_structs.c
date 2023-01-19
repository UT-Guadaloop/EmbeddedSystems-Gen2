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
//*****************************************************************************
//
// usb_serial_structs.c - Data structures defining this CDC USB device.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/usblib/msp432e4/usblib.h"
#include "ti/usblib/msp432e4/usbcdc.h"
#include "ti/usblib/msp432e4/usb-ids.h"
#include "ti/usblib/msp432e4/device/usbdevice.h"
#include "ti/usblib/msp432e4/device/usbdcdc.h"
#include "ti/usblib/msp432e4/device/usbdcomp.h"
#include "usb_structs.h"

//*****************************************************************************
//
// The languages supported by this device.
//
//*****************************************************************************
const uint8_t g_pui8LangDescriptor[] =
{
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

//*****************************************************************************
//
// The manufacturer string.
//
//*****************************************************************************
const uint8_t g_pui8ManufacturerString[] =
{
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
    't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};

//*****************************************************************************
//
// The product string.
//
//*****************************************************************************
const uint8_t g_pui8ProductString[] =
{
    2 + (16 * 2),
    USB_DTYPE_STRING,
    'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0,
    'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0, 'o', 0, 'r', 0, 't', 0
};

//*****************************************************************************
//
// The serial number string.
//
//*****************************************************************************
const uint8_t g_pui8SerialNumberString[] =
{
    2 + (8 * 2),
    USB_DTYPE_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

//*****************************************************************************
//
// The control interface description string.
//
//*****************************************************************************
const uint8_t g_pui8ControlInterfaceString[] =
{
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'A', 0, 'C', 0, 'M', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0,
    'r', 0, 'o', 0, 'l', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

//*****************************************************************************
//
// The configuration description string.
//
//*****************************************************************************
const uint8_t g_pui8ConfigString[] =
{
    2 + (26 * 2),
    USB_DTYPE_STRING,
    'S', 0, 'e', 0, 'l', 0, 'f', 0, ' ', 0, 'P', 0, 'o', 0, 'w', 0,
    'e', 0, 'r', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0,
    'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0,
    'o', 0, 'n', 0
};

//*****************************************************************************
//
// The descriptor string table.
//
//*****************************************************************************
const uint8_t * const g_pui8StringDescriptors[] =
{
    g_pui8LangDescriptor,
    g_pui8ManufacturerString,
    g_pui8ProductString,
    g_pui8SerialNumberString,
    g_pui8ControlInterfaceString,
    g_pui8ConfigString
};

#define NUM_STRING_DESCRIPTORS (sizeof(g_pui8StringDescriptors) /             \
                                sizeof(uint8_t *))

//*****************************************************************************
//
// The CDC device initialization and customization structures. In this case,
// we are using USBBuffers between the CDC device class driver and the
// application code. The function pointers and callback data values are set
// to insert a buffer in each of the data channels, transmit and receive.
//
// With the buffer in place, the CDC channel callback is set to the relevant
// channel function and the callback data is set to point to the channel
// instance data. The buffer, in turn, has its callback set to the application
// function and the callback data set to our CDC instance structure.
//
//*****************************************************************************
tUSBDCDCDevice g_psCDCDevice[NUM_SERIAL_DEVICES] =
{
    {
        USB_VID_TI_1CBE,
        USB_PID_SERIAL,
        0,
        USB_CONF_ATTR_SELF_PWR,
        ControlHandler,
        (void *)&g_psCDCDevice[0],
        USBBufferEventCallback,
        (void *)&g_psRxBuffer[0],
        USBBufferEventCallback,
        (void *)&g_psTxBuffer[0],
        g_pui8StringDescriptors,
        NUM_STRING_DESCRIPTORS
    },
    {
        USB_VID_TI_1CBE,
        USB_PID_SERIAL,
        0,
        USB_CONF_ATTR_SELF_PWR,
        ControlHandler,
        (void *)&g_psCDCDevice[1],
        USBBufferEventCallback,
        (void *)&g_psRxBuffer[1],
        USBBufferEventCallback,
        (void *)&g_psTxBuffer[1],
        g_pui8StringDescriptors,
        NUM_STRING_DESCRIPTORS
    }
};

//*****************************************************************************
//
// Receive buffer (from the USB perspective).
//
//*****************************************************************************
uint8_t g_ppui8USBRxBuffer[NUM_SERIAL_DEVICES][UART_BUFFER_SIZE];
tUSBBuffer g_psRxBuffer[NUM_SERIAL_DEVICES] =
{
    {
        false,                          // This is a receive buffer.
        RxHandlerEcho,                  // pfnCallback
        (void *)&g_psCDCDevice[0],      // Callback data is our device pointer.
        USBDCDCPacketRead,              // pfnTransfer
        USBDCDCRxPacketAvailable,       // pfnAvailable
        (void *)&g_psCDCDevice[0],      // pvHandle
        g_ppui8USBRxBuffer[0],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
    },
    {
        false,                          // This is a receive buffer.
        RxHandlerCmd,                   // pfnCallback
        (void *)&g_psCDCDevice[1],      // Callback data is our device pointer.
        USBDCDCPacketRead,              // pfnTransfer
        USBDCDCRxPacketAvailable,       // pfnAvailable
        (void *)&g_psCDCDevice[1],      // pvHandle
        g_ppui8USBRxBuffer[1],          // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
    }
};

//*****************************************************************************
//
// Transmit buffer (from the USB perspective).
//
//*****************************************************************************
uint8_t g_ppcUSBTxBuffer[NUM_SERIAL_DEVICES][UART_BUFFER_SIZE];
tUSBBuffer g_psTxBuffer[NUM_SERIAL_DEVICES] =
{
    {
        true,                           // This is a transmit buffer.
        TxHandlerEcho,                  // pfnCallback
        (void *)&g_psCDCDevice[0],      // Callback data is our device pointer.
        USBDCDCPacketWrite,             // pfnTransfer
        USBDCDCTxPacketAvailable,       // pfnAvailable
        (void *)&g_psCDCDevice[0],      // pvHandle
        g_ppcUSBTxBuffer[0],            // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
    },
    {
        true,                           // This is a transmit buffer.
        TxHandlerCmd,                   // pfnCallback
        (void *)&g_psCDCDevice[1],      // Callback data is our device pointer.
        USBDCDCPacketWrite,             // pfnTransfer
        USBDCDCTxPacketAvailable,       // pfnAvailable
        (void *)&g_psCDCDevice[1],      // pvHandle
        g_ppcUSBTxBuffer[1],            // pcBuffer
        UART_BUFFER_SIZE,               // ulBufferSize
    }
};

//****************************************************************************
//
// The memory allocated to hold the composite descriptor that is created by
// the call to USBDCompositeInit().
//
//****************************************************************************
uint8_t g_pui8DescriptorData[DESCRIPTOR_DATA_SIZE];

tCompositeEntry g_psCompEntries[NUM_SERIAL_DEVICES];

//****************************************************************************
//
// Allocate the Device Data for the top level composite device class.
//
//****************************************************************************
tUSBDCompositeDevice g_sCompDevice =
{
    //
    // Stellaris VID.
    //
    USB_VID_TI_1CBE,

    //
    // Stellaris PID for composite serial device.
    //
    USB_PID_COMP_SERIAL,

    //
    // This is in 2mA increments so 500mA.
    //
    250,

    //
    // Bus powered device.
    //
    USB_CONF_ATTR_BUS_PWR,

    //
    // There is no need for a default composite event handler.
    //
    0,

    //
    // The string table.
    //
    g_pui8StringDescriptors,
    NUM_STRING_DESCRIPTORS,

    //
    // The Composite device array.
    //
    2,
    g_psCompEntries
};
