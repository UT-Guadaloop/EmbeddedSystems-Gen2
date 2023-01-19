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
// usb_dev_mouse.c Main routines for device side mouse
//
//****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/usblib/msp432e4/usblib.h"
#include "ti/usblib/msp432e4/usbhid.h"
#include "ti/usblib/msp432e4/device/usbdevice.h"
#include "ti/usblib/msp432e4/device/usbdhid.h"
#include "ti/usblib/msp432e4/device/usbdhidmouse.h"
#include "pinout.h"
#include "usb_mouse_structs.h"
#include "uartstdio.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB HID Mouse Device (usb_dev_mouse)</h1>
//!
//! This example application turns the evaluation board into a USB mouse device
//! supporting the Human Interface Device class.  When the example is run
//! it functions as a mouse on the host.  It causes the mouse pointer to move
//! in a circular pattern on the screen.  To re-gain control of the mouse,
//! unplug USB.
//
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

//*****************************************************************************
//
// This global indicates whether or not we are connected to a USB host.
//
//*****************************************************************************
volatile bool g_bConnected = false;

//*****************************************************************************
//
// This global indicates whether or not the USB bus is currently in the suspend
// state.
//
//*****************************************************************************
volatile bool g_bSuspended = false;


//*****************************************************************************
//
// Global system tick counter holds elapsed time since the application started
// expressed in 100ths of a second.
//
//*****************************************************************************
volatile uint32_t g_ui32SysTickCount;

//*****************************************************************************
//
// The HID mouse input report data elements that are sent to the host.
//
//*****************************************************************************
static tMouseDescriptorReport sMouseReport = { 0, 0, 0 };


//*****************************************************************************
//
// Lookup table for mouse position values.  "const" indicates it will be stored
// in flash.
//
//*****************************************************************************
static const int16_t g_ppi16TableSinCosLookUp[93][2] = {
    {0,200},\
    {14,200},\
    {28,198},\
    {42,196},\
    {55,192},\
    {68,188},\
    {81,183},\
    {94,177},\
    {106,170},\
    {118,162},\
    {129,153},\
    {139,144},\
    {149,134},\
    {158,123},\
    {166,112},\
    {173,100},\
    {180,88},\
    {185,75},\
    {190,62},\
    {194,48},\
    {197,35},\
    {199,21},\
    {200,7},\
    {200,-7},\
    {199,-21},\
    {197,-35},\
    {194,-48},\
    {190,-62},\
    {185,-75},\
    {180,-88},\
    {173,-100},\
    {166,-112},\
    {158,-123},\
    {149,-134},\
    {139,-144},\
    {129,-153},\
    {118,-162},\
    {106,-170},\
    {94,-177},\
    {81,-183},\
    {68,-188},\
    {55,-192},\
    {42,-196},\
    {28,-198},\
    {14,-200},\
    {0,-200},\
    {-14,-200},\
    {-28,-198},\
    {-42,-196},\
    {-55,-192},\
    {-68,-188},\
    {-81,-183},\
    {-94,-177},\
    {-106,-170},\
    {-118,-162},\
    {-129,-153},\
    {-139,-144},\
    {-149,-134},\
    {-158,-123},\
    {-166,-112},\
    {-173,-100},\
    {-180,-88},\
    {-185,-75},\
    {-190,-62},\
    {-194,-48},\
    {-197,-35},\
    {-199,-21},\
    {-200,-7},\
    {-200,7},\
    {-199,21},\
    {-197,35},\
    {-194,48},\
    {-190,62},\
    {-185,75},\
    {-180,88},\
    {-173,100},\
    {-166,112},\
    {-158,123},\
    {-149,134},\
    {-139,144},\
    {-129,153},\
    {-118,162},\
    {-106,170},\
    {-94,177},\
    {-81,183},\
    {-68,188},\
    {-55,192},\
    {-42,196},\
    {-28,198},\
    {-14,200},\
    {0,200}
};



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
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

//*****************************************************************************
//
// This is the interrupt handler for the SysTick interrupt.  It is used to
// update our local tick count which, in turn, is used to check for transmit
// timeouts.
//
//*****************************************************************************
void  //akb do I need this
SysTick_Handler(void)
{
    g_ui32SysTickCount++;
}


//*****************************************************************************
//
// Handles asynchronous events from the HID mouse driver.
//
// \param pvCBData is the event callback pointer provided during
// USBDHIDmouseInit().  This is a pointer to our mouse device structure
// (&g_smouseDevice).
// \param ui32Event identifies the event we are being called back for.
// \param ui32MsgData is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the HID mouse driver to inform the application
// of particular asynchronous events related to operation of the mouse HID
// device.
//
// \return Returns 0 in all cases.
//
//*****************************************************************************
uint32_t
MouseHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgData,
                void *pvMsgData)
{
    switch (ui32Event)
    {
        //
        // The host has connected to us and configured the device.
        //
        case USB_EVENT_CONNECTED:
        {

            g_bConnected = true;
            break;
        }

        //
        // The host has disconnected from us.
        //
        case USB_EVENT_DISCONNECTED:
        {

            g_bConnected = false;
            break;
        }

        //
        // This event occurs every time the host acknowledges transmission
        // of a report.  It is to return to the idle state so that a new report
        // can be sent to the host.
        //
        case USB_EVENT_TX_COMPLETE:
        {
            //
            // We finished sending something.
            //
            break;
        }

        //
        // This event indicates that the host has suspended the USB bus.
        //
        case USB_EVENT_SUSPEND:
        {
            //
            // Go to the suspended state.
            //

            g_bSuspended = true;

            //
            // Turn off LED
            //
            ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

            break;
        }

        //
        // This event signals that the host has resumed signaling on the bus.
        //
        case USB_EVENT_RESUME:
        {
            //
            // Go back to the idle state.
            //
            g_bConnected = true;
            g_bSuspended = false;

            break;
        }

        //
        // We ignore all other events.
        //
        default:
        {
            break;
        }
    }

    return(0);
}

//*****************************************************************************
//
// This is the main loop that runs the application.
//
//*****************************************************************************
int
main(void)
{
    uint8_t ui8Index = 1;
    uint_fast32_t ui32LastTickCount;
    uint32_t ui32PLLRate;
    uint8_t g_ui8Updates = 0;
#ifdef USE_ULPI
    uint32_t ui32ULPI;
#endif

    //
    // Run from the PLL at 120 MHz.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_480), 120000000);

    //
    // Configure the device pins for this board.
    //
    PinoutSet(false, true);


    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //
    // Enable the GPIO pin for the Blue LED (PF2).
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);


    //
    // Not configured initially.
    //
    g_bConnected = false;
    g_bSuspended = false;


    //
    // Initialize the USB stack for device mode.
    //
    USBStackModeSet(0, eUSBModeDevice, 0);

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
    // Set the system tick to fire 100 times per second.
    //
    SysTickPeriodSet(g_ui32SysClock / SYSTICKS_PER_SECOND);
    SysTickIntEnable();
    SysTickEnable();

    //
    // Pass our device information to the USB HID device class driver,
    // initialize the USB controller and connect the device to the bus.
    //
    USBDHIDMouseInit(0, &g_sMouseDevice);

    //
    // The main loop starts here.  We begin by waiting for a host connection
    // then drop into the main mouse handling section.  If the host
    // disconnects, we return to the top and wait for a new connection.
    //
    while(1)
    {

        //
        // Wait here until USB device is connected to a host.
        //
        while(!g_bConnected)
        {
        }

        //
        // Keep sending mouse data for as long as we are connected to the host.
        //
        while(g_bConnected)
        {

            //
            // Remember the current time.
            //
            ui32LastTickCount = g_ui32SysTickCount;

            //
            // Build the report - position in X-axis
            //
            sMouseReport.dX =
                  (g_ppi16TableSinCosLookUp[ui8Index][0] -
                       g_ppi16TableSinCosLookUp[ui8Index - 1][0]) >> 1;
            //
            // Build the report - position in Y-axis
            //
            sMouseReport.dY =
                  (g_ppi16TableSinCosLookUp[ui8Index][1] -
                        g_ppi16TableSinCosLookUp[ui8Index - 1][1]) >> 1;

            // Send the report
            USBDHIDMouseStateChange(&g_sMouseDevice, sMouseReport.dX,
                                     sMouseReport.dY, 0);

            //
            //reset position index
            //
            if (ui8Index++ >= 90)
            {
                 ui8Index = 1;
            }

            //
            // Wait for at least 1 system tick to have gone by before
            // updating the mouse position.
            //

            while(g_ui32SysTickCount == ui32LastTickCount)
            {
            }

            //
            // Turn on the green LED
            //
            ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
            //
            // Limit the blink rate of the LED.
            //
             if(g_ui8Updates++ == 20)
             {
                 //
                 // Turn off the green LED.
                 //
                 ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~GPIO_PIN_0);

                 //
                 // Reset the update count.
                 //
                 g_ui8Updates = 0;
             }
        }
    }

}
