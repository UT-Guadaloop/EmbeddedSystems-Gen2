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
//****************************************************************************
//
// usb_dev_cdc_serial.c - Main routines for the single USB CDC serial example.
//
//****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/usblib/msp432e4/usblib.h"
#include "ti/usblib/msp432e4/usbcdc.h"
#include "ti/usblib/msp432e4/usb-ids.h"
#include "ti/usblib/msp432e4/device/usbdevice.h"
#include "ti/usblib/msp432e4/device/usbdcdc.h"
#include "ustdlib.h"
#include "pinout.h"
#include "usb_structs.h"

//****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB CDC Serial Device (usb_dev_cdc_serial)</h1>
//!
//! This example application turns the evaluation kit into a CDC device
//! when connected to the USB host system.  The application
//! supports the USB Communication Device Class, Abstract Control Model to
//! by interacting with the USB host via a virtual COM port.  For this example,
//! the evaluation kit will enumerate as a CDC device with one virtual
//! serial port.
//!
//! Once the virtual COM port is opened, the user should see a continuous
//! display of "Press any key" command.  When any key is pressed, the
//! CDC device will then respond back with 'Received the key press'.
//!
//! A Windows INF driver file for the device is provided 
//! in the <simplelink_install_folder>/tools/usblib/windows_drivers directory.
//!  This INF contains information required to install CDC driver on the 
//! Windows OS>.
//!
//! Set baudrate = 115200, 8:1
//!
//*****************************************************************************

//****************************************************************************
//
// The system tick rate expressed both as ticks per second and a millisecond
// period.
//
//****************************************************************************
#define SYSTICKS_PER_SECOND 100
#define SYSTICK_PERIOD_MS (1000 / SYSTICKS_PER_SECOND)

//*****************************************************************************
//
// Variable to remember our clock frequency
//
//*****************************************************************************
uint32_t g_ui32SysClock = 0;

//****************************************************************************
//
// Variables tracking transmit and receive counts.
//
//****************************************************************************
volatile uint32_t g_ui32UARTTxCount = 0;
volatile uint32_t g_ui32UARTRxCount = 0;

//****************************************************************************
//
// Default line coding settings for the redirected UART.
//
//****************************************************************************
#define DEFAULT_BIT_RATE        115200
#define DEFAULT_UART_CONFIG     (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | \
                                 UART_CONFIG_STOP_ONE)

//****************************************************************************
//
// GPIO peripherals and pins muxed with the redirected UART.  These will
// depend upon the IC in use and the UART selected in UART0_BASE.  Be careful
// that these settings all agree with the hardware you are using.
//
//****************************************************************************
#define TX_GPIO_BASE            GPIO_PORTA_BASE
#define TX_GPIO_PERIPH          SYSCTL_PERIPH_GPIOA
#define TX_GPIO_PIN             GPIO_PIN_1

#define RX_GPIO_BASE            GPIO_PORTA_BASE
#define RX_GPIO_PERIPH          SYSCTL_PERIPH_GPIOA
#define RX_GPIO_PIN             GPIO_PIN_0

//****************************************************************************
//
// The LED control macros.
//
//****************************************************************************
#define LEDOn()  ROM_GPIOPinWrite(CLP_D1_PORT, CLP_D1_PIN, CLP_D1_PIN);

#define LEDOff() ROM_GPIOPinWrite(CLP_D1_PORT, CLP_D1_PIN, 0)
#define LEDToggle()                                                         \
        ROM_GPIOPinWrite(CLP_D1_PORT, CLP_D1_PIN,                           \
                         (ROM_GPIOPinRead(CLP_D1_PORT, CLP_D1_PIN) ^        \
                          CLP_D1_PIN));


//****************************************************************************
//
// Defines the size of the buffer that holds the command line.
//
//****************************************************************************
#define CMD_BUF_SIZE            256

//****************************************************************************
//
// The buffer that holds the command line.
//
//****************************************************************************
static char g_pcCmdBuf[CMD_BUF_SIZE];
static uint32_t ui32CmdIdx;

//****************************************************************************
//
// Flag indicating whether or not we are currently sending a Break condition.
//
//****************************************************************************
//static bool g_bSendingBreak = false;

//****************************************************************************
//
// Global system tick counter
//
//****************************************************************************
volatile uint32_t g_ui32SysTickCount = 0;


//****************************************************************************
//
// Flags used to pass commands from interrupt context to the main loop.
//
//****************************************************************************
#define COMMAND_RECEIVED        0x00000001
#define COMMAND_TRANSMIT_COMPLETE  0x00000002


volatile uint32_t g_ui32Flags = 0;

//****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//****************************************************************************
static volatile bool g_bUSBConfigured = false;

//****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1)
    {
    }
}
#endif

extern void USB0_IRQDeviceHandler(void);
//This dummy handler created as a hack for USB interrupt in
//startup file.  When the USB interrupt has attribute of 'weak'
//and usb.lib is linked to project, the interrupt handler in
//../device/usbdhandler.c file is routed
//to default handler.  The 'weak' attribute only works for
//dynamic libraries and not static library
void USB0_IRQHandler(void)
{

    USB0_IRQDeviceHandler();
}


//****************************************************************************
//
// Interrupt handler for the system tick counter.
//
//****************************************************************************
void
SysTick_Handler(void)
{
    //
    // Update our system time.
    //
    g_ui32SysTickCount++;
}



//****************************************************************************
//
// Set the state of the RS232 RTS and DTR signals.  Handshaking is not
// supported so this request will be ignored.
//
//****************************************************************************
static void
SetControlLineState(unsigned short usState)
{
}

//****************************************************************************
//
// Set the communication parameters to use on the UART.
//
//****************************************************************************
static bool
SetLineCoding(tLineCoding *psLineCoding)
{
    //
    //user to enter customized data here
    //
    //
    // Let the caller know if we had a problem or not.
    //
    return(0);
}

//****************************************************************************
//
// Get the communication parameters in use on the CDC terminal.
//
//****************************************************************************
static void
GetLineCoding(tLineCoding *psLineCoding)
{
    //
    // Get the current line coding set for the CDC terminal.
    //
    psLineCoding->ui32Rate = DEFAULT_BIT_RATE;
    psLineCoding->ui8Databits = 8;
    psLineCoding->ui8Parity = USB_CDC_PARITY_NONE;
    psLineCoding->ui8Stop = USB_CDC_STOP_BITS_1;

}


//****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the
// device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//****************************************************************************
uint32_t
ControlHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{

    //
    // Which event are we being asked to process?
    //
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
        {
            g_bUSBConfigured = true;
            g_ui32Flags |= COMMAND_TRANSMIT_COMPLETE;

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_psTxBuffer);
            USBBufferFlush(&g_psRxBuffer);

            break;
        }

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
        {
            g_bUSBConfigured = false;
            break;
        }

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
        {
            GetLineCoding(pvMsgData);
            break;
        }

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_LINE_CODING:
        {
            SetLineCoding(pvMsgData);
            break;
        }

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
        {
            SetControlLineState((unsigned short)ui32MsgValue);
            break;
        }

        //
        // Send a break condition on the serial line.
        //
        case USBD_CDC_EVENT_SEND_BREAK:
        {
            break;
        }

        //
        // Clear the break condition on the serial line.
        //
        case USBD_CDC_EVENT_CLEAR_BREAK:
        {
            break;
        }

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
        {
            break;
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
        {
            break;
        }
    }

    return(0);
}



//****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ui32CBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//****************************************************************************
uint32_t
TxHandlerCmd(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
             void *pvMsgData)
{
    //
    // Which event have we been sent?
    //
    switch(ui32Event)
    {
        case USB_EVENT_TX_COMPLETE:
        {
            //
            // Clear the flag
            //
            g_ui32Flags |= COMMAND_TRANSMIT_COMPLETE;

            break;
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
        {
            break;
        }
    }
    return(0);
}



//****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ui32CBData is the client-supplied callback data value for this
// channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//****************************************************************************

uint32_t
RxHandlerCmd(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
             void *pvMsgData)
{
    const tUSBDCDCDevice *psCDCDevice;
    const tUSBBuffer *pBufferRx;

    switch(ui32Event)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            //
            // Create a device pointer.
            //
            psCDCDevice = (const tUSBDCDCDevice *)pvCBData;
            pBufferRx = (const tUSBBuffer *)psCDCDevice->pvRxCBData;

            //
            // Keep reading characters as long as there are more to receive.
            //

            if(USBBufferRead(pBufferRx,
                            (uint8_t *)&g_pcCmdBuf[ui32CmdIdx], 1))
            {


                //
                // Indicate that a command has been received.
                //
                g_ui32Flags |= COMMAND_RECEIVED;

                g_pcCmdBuf[ui32CmdIdx] = 0;

                ui32CmdIdx = 0;

            }


            break;
        }

        //
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            //
            // Get the number of bytes in the buffer and add 1 if some data
            // still has to clear the transmitter.
            //
            return(0);
        }

        //
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be
        // sending this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
        {
            break;
        }
    }

    return(0);
}


//****************************************************************************
//
// This is the main application entry function.
//
//****************************************************************************
int
main(void)
{

    uint32_t ui32PLLRate;
    char cOutString[32];
    char cOutString2[32];
#ifdef USE_ULPI
    uint32_t ui32ULPI;
#endif
    ui32CmdIdx = 0;

    //
    // String to output to the user terminal
    //
    strcpy(cOutString,"Press any key.\r");

    //
    // Response string indicating data was received from user
    //
    strcpy(cOutString2,"\r\n\r\n\r\nReceived the key press.\r\n");


    //
    // Run from the PLL at 120 MHz.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    //
    // Not configured initially.
    //
    g_bUSBConfigured = false;


    //
    // Enable the peripherals used in this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);

    //
    // Configure the device pins.
    //
    PinoutSet(false, true);



    //
    // Enable the system tick.
    //
    SysTickPeriodSet(g_ui32SysClock / SYSTICKS_PER_SECOND);
    SysTickIntEnable();
    SysTickEnable();

    //
    // Initialize the transmit and receive buffers for first serial device.
    //
    USBBufferInit(&g_psTxBuffer);
    USBBufferInit(&g_psRxBuffer);

    //
    // Initialize the serial port
    //
    USBDCDCCompositeInit(0, &g_psCDCDevice, 0);

    //
    // Tell the USB library the CPU clock and the PLL frequency.
    //
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &ui32PLLRate);
    USBDCDFeatureSet(0, USBLIB_FEATURE_CPUCLK, &g_ui32SysClock);
    USBDCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &ui32PLLRate);

#ifdef USE_ULPI
    //
    // Tell the USB library to use ULPI interface
    //
    ui32ULPI = USBLIB_FEATURE_ULPI_HS;
    USBDCDFeatureSet(0, USBLIB_FEATURE_USBULPI, &ui32ULPI);
#endif

    //
    // Initial serial device and pass the device information to the USB library and place the device
    // on the bus.
    //
    USBDCDCInit(0, &g_psCDCDevice);

    //
    // Main application loop.
    //
    while(1)
    {
        //
        // Device has been enumerated by host
        //
        if(g_bUSBConfigured)
        {
            //
            // flag for data transmission completed to host is set and
            // data received flag has been cleared
            //
            if(g_ui32Flags & COMMAND_TRANSMIT_COMPLETE & ~COMMAND_RECEIVED)
            {
                //
                //Clear the data transmission flag
                //
                 g_ui32Flags &= ~COMMAND_TRANSMIT_COMPLETE;

                 //
                 // Send data to host
                 //
                 USBBufferWrite((tUSBBuffer *)&g_psTxBuffer, (uint8_t*)cOutString, strlen(cOutString));
            }
            //
            // Received data flag is set and data transmission complete flag has been cleared
            //
            if(g_ui32Flags & COMMAND_RECEIVED & ~COMMAND_TRANSMIT_COMPLETE)
            {
                //
                // Clear the flag
                //
                g_ui32Flags &= ~COMMAND_RECEIVED;
                g_ui32Flags &= ~COMMAND_TRANSMIT_COMPLETE;


                //
                // Send data to host acknowledging the reception of data from host
                //
                USBBufferWrite((tUSBBuffer *)&g_psTxBuffer, (uint8_t*)cOutString2, strlen(cOutString2));

            }
        }
    }
}
