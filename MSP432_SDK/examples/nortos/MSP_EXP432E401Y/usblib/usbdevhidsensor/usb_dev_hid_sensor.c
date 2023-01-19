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
// usb_dev_hid_sensor.c - Main routines for the HID Sensor example.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/usblib/msp432e4/usblib.h"
#include "ti/usblib/msp432e4/usbhid.h"
#include "ti/usblib/msp432e4/device/usbdevice.h"
#include "ti/usblib/msp432e4/device/usbdhid.h"
#include "ti/usblib/msp432e4/device/usbdhidsensor.h"

#include "pinout.h"
#include "usb_sensor_structs.h"
#include "uartstdio.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB HID Sensor Device (usb_dev_hid_sensor)</h1>
//!
//! This example application turns the evaluation board into a USB Temperature
//! Sensor/HID device supporting the Human Interface Device class.  When the
//! program is run, the sensor/HID device will send its internal temperature in
//! degrees C along with the device's state and event data.  The generic HID
//! application tool 'HidDemo_Tool' can be used to display the data.  The tool
//! can be found in the following location in the SDK package:
//!
//! C:\ti\simplelink_msp432e4_sdk_xx_xx_xx\tools\examples\usb_dev_hid_sensor
//!
//! The device's temperature, state and event is based on the sensor values
//! specified in the HID Report descriptor.
//!
//! NOTE: The HidDemo_Tool can only be used to communicate with the sensor
//!       device if it is classified as a Generic HID device.  This example
//!       sets the sensor device as a generic HID device by setting the Usage
//!       Page in the HID Report descriptor as 'Vendor Defined' and not as
//!       USB_HID_SENSOR.   
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
// The HID sensor report that is returned to the host.
//
//*****************************************************************************
static tSensorTemperatureReport sReport;


//*****************************************************************************
//
// An activity counter to slow the LED blink down to a visible rate.
//
//*****************************************************************************
static uint32_t g_ui32Updates;

//*****************************************************************************
//
// Variable to remember our clock frequency
//
//*****************************************************************************
uint32_t g_ui32SysClock = 0;

//*****************************************************************************
//
// Global system tick counter holds elapsed time since the application started
// expressed in 100ths of a second.
//
//*****************************************************************************
volatile uint32_t g_ui32SysTickCount;

//*****************************************************************************
//
// This enumeration holds the various states that the temperature sensor
// can be in during normal operation.
//
//*****************************************************************************
volatile enum
{
    //
    // Not yet configured.
    //
    eStateNotConfigured,

    //
    // Connected and not waiting on data to be sent.
    //
    eStateIdle,

    //
    // Suspended.
    //
    eStateSuspend,

    //
    // Connected and waiting on data to be sent out.
    //
    eStateSending
}g_iSensorState;


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
void
SysTick_Handler(void)
{
    g_ui32SysTickCount++;
}


//*****************************************************************************
//
// Handles asynchronous events from the HID sensor driver.
//
// \param pvCBData is the event callback pointer provided during
// USBDHIDSensorInit().  This is a pointer to our sensor device structure
// (&g_sSensorDevice).
// \param ui32Event identifies the event we are being called back for.
// \param ui32MsgData is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the HID sensor driver to inform the application
// of particular asynchronous events related to operation of the sensor HID
// device.
//
// \return Returns 0 in all cases.
//
//*****************************************************************************
uint32_t
SensorHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgData,
                void *pvMsgData)
{
    switch (ui32Event)
    {
        //
        // The host has connected to us and configured the device.
        //
        case USB_EVENT_CONNECTED:
        {
            g_iSensorState = eStateIdle;

            //
            // Update the status.
            //
            UARTprintf("\nHost Connected...\n");

            break;
        }

        //
        // The host has disconnected from us.
        //
        case USB_EVENT_DISCONNECTED:
        {
            g_iSensorState = eStateNotConfigured;

            //
            // Update the status.
            //
            UARTprintf("\nHost Disconnected...\n");

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
            // Enter the idle state since we finished sending something.
            //
            g_iSensorState = eStateIdle;

            //LED port and pin
            ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

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
            g_iSensorState = eStateSuspend;

            //
            // Suspended.
            //
            UARTprintf("\nBus Suspended\n");

            // LED pin
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
            g_iSensorState = eStateIdle;

            //
            // Resume signaled.
            //
            UARTprintf("\nBus Resume\n");

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
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{

    //
    // Enable UART0
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);


    //
    // Initialize the UART for console I/O.
    // Configure the UART for 115200 bps 8-N-1 format
    //
    UARTStdioConfig(0, 115200, g_ui32SysClock);
}

//*****************************************************************************
//
// Initialize the ADC inputs used by the sensor device.  This example uses
// the ADC pins on Port E pins 1, 2, and 3(AIN0-2).
//
//*****************************************************************************
void
ADCInit(void)
{

    //
    // Enable the clock to ADC-0 and wait for it to be ready
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    //
    // Configure Sequencer 3 to sample a single analog channel : Internal
    // Temperature Sensor
    //
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_TS | ADC_CTL_IE |
                                 ADC_CTL_END);

    //
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion
    //
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    MAP_ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    MAP_ADCIntClear(ADC0_BASE, 3);


}

//*****************************************************************************
//
// This is the main loop that runs the application.
//
//*****************************************************************************
int
main(void)
{

    bool bUpdate;

    uint32_t getADCValue[1];
    uint32_t tempSenseinC;
    uint32_t tempSenseinF;
    uint32_t ui32PLLRate;
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
    // Open UART0 and show the application name on the UART.
    //
    ConfigureUART();

    UARTprintf("USB Temperature Sensor device example\n");
    UARTprintf("---------------------------------\n\n");

    //
    // Not configured initially.
    //
    g_iSensorState = eStateNotConfigured;


    //
    // Initialize the ADC channels.
    //
    ADCInit();

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
    USBDHIDSensorInit(0, &g_sSensorDevice);

    //
    // The main loop starts here.  We begin by waiting for a host connection
    // then drop into the main sensor handling section.  If the host
    // disconnects, we return to the top and wait for a new connection.
    //
    while(1)
    {


        if(g_iSensorState == eStateIdle)
        {
            //
            // No update by default.
            //
            bUpdate = false;

            /* Trigger the ADC conversion. */
            MAP_ADCProcessorTrigger(ADC0_BASE, 3);

            /* Wait for conversion to be completed. */
            while(!MAP_ADCIntStatus(ADC0_BASE, 3, false))
            {
            }

            /* Clear the ADC interrupt flag. */
            MAP_ADCIntClear(ADC0_BASE, 3);

            /* Read ADC Value. */
            MAP_ADCSequenceDataGet(ADC0_BASE, 3, getADCValue);

            /* Convert raw ADC value to degree C and F */
            tempSenseinC = (1475*4096 - (75 * 33 * getADCValue[0]))/ 40960;

            /* store the temperature value as part of the input HID report
             * data.
             */
            sReport.i16Temp = (uint16_t)tempSenseinC;

            /* convert temperature to degree F*/
            tempSenseinF = (tempSenseinC * 9)/5 + 32;

            /* set the sensor state to ready as part of the input HID report data*/
            sReport.ui8SensorState = 1;  //sensor state is ready;

            /* set the sensor event to data updated as part of the input HID report data*/
            sReport.ui8SensorEvent = 19; //sensor event is data updated;


            /* Display the Temp Sense digital value on the console. */
            UARTprintf("\033[2J\033[H");
            UARTprintf("Raw Temp Sense = %4d\n", getADCValue[0]);
            UARTprintf("Temp Sense (C) = %3d\n", tempSenseinC);
            UARTprintf("Temp Sense (F) = %3d\n", tempSenseinF);


            /* Delay the next sampling */
            MAP_SysCtlDelay(g_ui32SysClock / 12);
            bUpdate = true;

        }
        //
        // Send the report if there was an update.
        //
        if(bUpdate)
        {
            ///
            // Send the input report consisting of temp, state and
            // event
            //
            USBDHIDSensorSendReport(&g_sSensorDevice, &sReport,
                                     sizeof(sReport));

            //
            // Now sending data but protect this from an interrupt since
            // it can change in interrupt context as well.
            //
            IntMasterDisable();
            g_iSensorState = eStateSending;
            IntMasterEnable();

            ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
            //
            // Limit the blink rate of the LED.
            //
            if(g_ui32Updates++ == 4)
            {
                //
                // Turn off the green LED.
                //
                ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~GPIO_PIN_0);

                //
                // Reset the update count.
                //
                g_ui32Updates = 0;
            }
        }

    }



}
