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
// usb_host_serial.c - An example that supports CDC serial devices.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/usblib/msp432e4/usblib.h"
#include "ti/usblib/msp432e4/usbcdc.h"
#include "ti/usblib/msp432e4/host/usbhost.h"
#include "ti/usblib/msp432e4/host/usbhcdc.h"
#include "ti/usblib/msp432e4/host/usbhcdcserial.h"
#include "uartstdio.h"
#include "pinout.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB Host CDC Serial example(usb_host_serial)</h1>
//!
//! This example application demonstrates data transfer between a CDC Host and
//! a CDC Serial Device using the MSP_EXP432E401Y evaluation kit.  
//! To demonstrate the data transfer the CDC device example, "usbDevCdcSerial",
//! should be run on 
//! another MSP-EXP432E401Y evaluation kit.   The data transferred from the
//! device is displayed on the UART terminal. 
//!
//! The example automates the transfer of data between host and device.
//! Upon initial connection of the CDC device, the UART terminal will display
//! a certain number of 'Press Any Key' prompts and when the application
//! sends the character 'j' to the device, the device replies back with
//! 'Received the key press'.
//!
//! UART running at 115,200, 8-N-1,
//! is used to display messages from this application.
//!
//!
//
//*****************************************************************************

//*****************************************************************************
//
// The number of SysTick ticks per second.
//
//*****************************************************************************
#define TICKS_PER_SECOND 100
#define MS_PER_SYSTICK (1000 / TICKS_PER_SECOND)

//*****************************************************************************
//
// Our running system tick counter and a global used to determine the time
// elapsed since last call to GetTickms().
//
//*****************************************************************************
uint32_t g_ui32SysTickCount;
uint32_t g_ui32LastTick;

//*****************************************************************************
//
// The size of the host controller's memory pool in bytes.  If high speed then
// memory size is set for 1024 bytes.  If full speed, memory size is 128
//
//*****************************************************************************
#ifdef USE_ULPI
#define HCD_MEMORY_SIZE         1024
#else
#define HCD_MEMORY_SIZE         128
#endif

//*****************************************************************************
//
// The bulk enpoint interface number
//
//*****************************************************************************
#define INTERFACE_1      1

//*****************************************************************************
//
// The memory pool to provide to the Host controller driver.
//
//*****************************************************************************
uint8_t g_pHCDPool[HCD_MEMORY_SIZE];


//*****************************************************************************
//
// Declare the USB Events driver interface.
//
//*****************************************************************************
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);

//*****************************************************************************
//
// The global that holds all of the host drivers in use in the application.
// In this case, only the CDC class is loaded.
//
//*****************************************************************************
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
    &g_sUSBCDCClassDriver,
    &g_sUSBEventDriver
};

//*****************************************************************************
//
// This global holds the number of class drivers in the g_ppHostClassDrivers
// list.
//
//*****************************************************************************
static const uint32_t g_ui32NumHostClassDrivers =
    sizeof(g_ppHostClassDrivers) / sizeof(tUSBHostClassDriver *);

//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.  In this application uDMA is only used for USB,
// so only the first 6 channels are needed.
//
//*****************************************************************************
#if defined(__ICCARM__)
#pragma data_alignment=1024
tDMAControlTable g_sDMAControlTable[6];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(g_sDMAControlTable, 1024)
tDMAControlTable g_sDMAControlTable[6];
#else
tDMAControlTable g_sDMAControlTable[6] __attribute__ ((aligned(1024)));
#endif


//*****************************************************************************
//
// The instance data for the CDC driver.
//
//*****************************************************************************
tUSBHCDCSerial *g_psCDCSerialInstance = 0;


//*****************************************************************************
//
// This enumerated type is used to hold the states of the CDC device.
//
//*****************************************************************************
enum
{
    //
    // No device is present.
    //
    STATE_NO_DEVICE,


    //
    // CDC device has been detected and needs to be initialized in the main
    // loop.
    //
    STATE_CDC_DEVICE_INIT,

    //
    // CDC Device is connected and waiting for events.
    //
    STATE_CDC_DEVICE_CONNECTED,

    //
    // An unsupported device has been attached.
    //
    STATE_UNKNOWN_DEVICE,

    //
    // A power fault has occurred.
    //
    STATE_POWER_FAULT  
}
g_eUSBState;


//*****************************************************************************
//
// The current USB operating mode - Host, Device or unknown.
//
//*****************************************************************************
tUSBMode g_eCurrentUSBMode;


//*****************************************************************************
// Counter that automates the sending of a character to the device at known
// intervals. When counter reaches 10, a character is sent to device
//
//*****************************************************************************

uint8_t  ui8DataCounter = 0;


extern void USB0_IRQOTGModeHandler(void);
//This dummy handler created as a hack for USB interrupt in
//startup file.  When the USB interrupt has attribute of 'weak'
//and usb.lib is linked to project, the interrupt handler in
//../device/usbdhandler.c file is routed
//to default handler.  The 'weak' attribute only works for
//dynamic libraries and not static library
void USB0_IRQHandler(void)
{

    USB0_IRQOTGModeHandler();

}

//*****************************************************************************
//
// This is the handler for this SysTick interrupt.
//
//*****************************************************************************
void
SysTick_Handler(void)
{
    //
    // Update our tick counter.
    //
    g_ui32SysTickCount++;
}

//*****************************************************************************
//
// This function returns the number of ticks since the last time this function
// was called.
//
//*****************************************************************************
uint32_t
GetTickms(void)
{
    uint32_t ui32RetVal;
    uint32_t ui32Saved;

    ui32RetVal = g_ui32SysTickCount;
    ui32Saved = ui32RetVal;

    if(ui32Saved > g_ui32LastTick)
    {
        ui32RetVal = ui32Saved - g_ui32LastTick;
    }
    else
    {
        ui32RetVal = g_ui32LastTick - ui32Saved;
    }

    //
    // This could miss a few milliseconds but the timings here are on a
    // much larger scale.
    //
    g_ui32LastTick = ui32Saved;

    //
    // Return the number of milliseconds since the last time this was called.
    //
    return(ui32RetVal * MS_PER_SYSTICK);
}

//*****************************************************************************
//
// This is the callback from the USB CDC device handler.
//
// \param pvCBData is ignored by this function.
// \param ui32Event is one of the valid events for a CDC device.
// \param ui32MsgParam is defined by the event that occurs.
// \param pvMsgData is a pointer to data that is defined by the event that
// occurs.
//
// This function will be called to inform the application when a CDC device has
// been plugged in or removed and any time data is sent or received.
//
// \return This function will return 0.
//
//*****************************************************************************
void
CDCSerialCallback(tUSBHCDCSerial *psCDCInstance, uint32_t ui32Event,
                 uint32_t ui32MsgParam, void *pvMsgData)
{
    unsigned char ucChar;
    uint8_t i;

    switch(ui32Event)
    {
        //
        // Data was detected.
        //
        case USBH_EVENT_RX_CDC_DATA:
        {

            //
            // parse through 64 bytes of data to display
            //
            for (i = 0; i<64; i++)
            {

               //
               // Retrieve the data value pointed to by i
               //
               ucChar = (unsigned char)USBHCDCProcessData(g_psCDCSerialInstance, i);

               //
               // Print the data sent by attached CDC device out to the UART.
               //
               UARTprintf("%c", ucChar);

            }

            //
            // Track the number of times data has been received from device
            //
            ui8DataCounter++;
            break;
        }
        case USBH_EVENT_TX_CDC_DATA:
        {

           //Nothing to process here since we don't care about the result of
           //sending the data to the device.

            break;
        }
    }
}

//*****************************************************************************
//
// This is the generic callback from host stack.
//
// \param pvData is actually a pointer to a tEventInfo structure.
//
// This function will be called to inform the application when a USB event has
// occurred that is outside those related to the CDC device.  At this
// point this is used to detect unsupported devices being inserted and removed.
// It is also used to inform the application when a power fault has occurred.
// This function is required when the g_USBGenericEventDriver is included in
// the host controller driver array that is passed in to the
// USBHCDRegisterDrivers() function.
//
// \return None.
//
//*****************************************************************************
void
USBHCDEvents(void *pvData)
{
    tEventInfo *pEventInfo;

    //
    // Cast this pointer to its actual type.
    //
    pEventInfo = (tEventInfo *)pvData;

    switch(pEventInfo->ui32Event)
    {
        //
        // New CDC device detected.
        //
        case USB_EVENT_CONNECTED:
        {
            //
            // See if this is a CDC device where control interface descriptor
            //  (interface 0) is defined as CDC class and interface protocol is
            // Common AT commands (value of 02)
            //
            if((USBHCDDevClass(pEventInfo->ui32Instance, 0) == USB_CLASS_CDC) &&
               (USBHCDDevProtocol(pEventInfo->ui32Instance, 0) ==
                       USB_CDC_PROTOCOL_V25TER))
            {
                //
                // Indicate that the CDC device has been detected.
                //
                UARTprintf("\nCDC Device Connected\n");

                //
                // Proceed to the STATE_CDC_DEVICE_INIT state so that the main
                // loop can finish initialized the CDC device since
                // USBCDCSerialInit() cannot be called from within a callback.
                //
                g_eUSBState = STATE_CDC_DEVICE_INIT;
            }

            break;
        }
        //
        // Unsupported device detected.
        //
        case USB_EVENT_UNKNOWN_CONNECTED:
        {
            UARTprintf("Unsupported Device Class (0x%02x) Connected.\n",
                       pEventInfo->ui32Instance);

            //
            // An unknown device was detected.
            //
            g_eUSBState = STATE_UNKNOWN_DEVICE;

            break;
        }
        //
        // Device has been unplugged.
        //
        case USB_EVENT_DISCONNECTED:
        {
            //
            // Indicate that the device has been disconnected.
            //
            UARTprintf("\nDevice Disconnected\n");

            //
            // Change the state so that the main loop knows that the device
            // is no longer present.
            //
            g_eUSBState = STATE_NO_DEVICE;

            break;
        }
        //
        // Power Fault has occurred. 
        //
        case USB_EVENT_POWER_FAULT:
        {
            UARTprintf("Power Fault\n");

            //
            // No power means no device is present.
            //
            g_eUSBState = STATE_POWER_FAULT;

            break;
        }

        default:
        {
            break;
        }
    }
}




int main(void)
{	
    uint32_t ui32SysClock;
    uint32_t ui32PLLRate;
    uint8_t  ui8SendArray[64];
    uint8_t ui8DataSize;
#ifdef USE_ULPI
    uint32_t ui32ULPI;
#endif

    //
    // Initially wait for device connection.
    //
    g_eUSBState = STATE_NO_DEVICE;

    //
    // Run from the PLL at 120 MHz.
    //

    ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN |
                                           SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_480), 120000000);

    //
    // Enable the pins and peripherals used by this example.
    //
    PinoutSet(0,1);

    //
    // Enable the UART and print a brief message.
    //
    UARTStdioConfig(0, 115200, ui32SysClock);
    UARTprintf("\033[2J\033[H");
    UARTprintf("CDC Serial Application\n");

    //
    // Configure SysTick for a 100Hz interrupt.
    //
    ROM_SysTickPeriodSet(ui32SysClock / TICKS_PER_SECOND);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();

    //
    // Enable the uDMA controller and set up the control table base.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(g_sDMAControlTable);

    //
    // Initialize the USB stack in host mode. No callback is needed at this
    // time.
    //
    USBStackModeSet(0, eUSBModeHost, 0);

    //
    // Register the host class drivers.
    //
    USBHCDRegisterDrivers(0, g_ppHostClassDrivers, g_ui32NumHostClassDrivers);

    //
    // Open an instance of the CDC driver.  The CDC device does not need
    // to be present at this time, this just saves a place for it and allows
    // the applications to be notified when a CDC device is present.
    //
    g_psCDCSerialInstance = USBHCDCSerialOpen(CDCSerialCallback);


    //
    // Initialize the power configuration. This sets the power enable signal
    // to be active high and does not enable the power fault.
    //
    USBHCDPowerConfigInit(0, USBHCD_VBUS_AUTO_HIGH | USBHCD_VBUS_FILTER);

    //
    // Tell the USB library the CPU clock and the PLL frequency.
    //
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &ui32PLLRate);
    USBHCDFeatureSet(0, USBLIB_FEATURE_CPUCLK, &ui32SysClock);
    USBHCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &ui32PLLRate);

#ifdef USE_ULPI
    //
    // Tell the USB library to use ULPI interface
    //
    ui32ULPI = USBLIB_FEATURE_ULPI_HS;
    USBHCDFeatureSet(0, USBLIB_FEATURE_USBULPI, &ui32ULPI);
#endif

    //
    // Initialize the USB controller for host operation.
    //
    USBHCDInit(0, g_pHCDPool, HCD_MEMORY_SIZE);

    //
    // The main loop for the application.
    //
    while(1)
    {
        //
        // Tell the OTG library code how much time has passed in
        // milliseconds since the last call.
        //
        USBOTGMain(GetTickms());

        switch(g_eUSBState)
        {
            //
            // This state is entered when the CDC device is first detected.
            //
            case STATE_CDC_DEVICE_INIT:
            {
                //
                // Initialize the newly connected CDC device.
                //
                USBHCDCSerialInit(g_psCDCSerialInstance);


                //
                // Proceed to the connected CDC device state.
                //
                g_eUSBState = STATE_CDC_DEVICE_CONNECTED;



                break;
            }


            case STATE_CDC_DEVICE_CONNECTED:
            {

                //
                // Start polling for data on interface 1
                //
                USBHCDCGetDataFromDevice(g_psCDCSerialInstance, INTERFACE_1);

                //
                // 10 sets of string of data has been received from device
                //
                if (ui8DataCounter == 10)
                {
                    //Restart counter
                    ui8DataCounter = 0;
                    //
                    // Press a key to acknowledge reception of data
                    //
                    ui8SendArray[0] = 'j';
                    ui8DataSize = 1;

                    //
                    // Send the key press to the device.
                    //
                    USBHCDCSendDataToDevice(g_psCDCSerialInstance, INTERFACE_1, ui8SendArray, ui8DataSize);
                }


                break;
            }

            case STATE_UNKNOWN_DEVICE:
            {
                //
                // Nothing to do as the device is unknown.
                //
                break;
            }

            case STATE_NO_DEVICE:
            {
                //
                // Nothing is currently done in the main loop when the CDC
                // device is not connected.
                //
                break;
            }

            default:
            {
                break;
            }
        }
    }
}
