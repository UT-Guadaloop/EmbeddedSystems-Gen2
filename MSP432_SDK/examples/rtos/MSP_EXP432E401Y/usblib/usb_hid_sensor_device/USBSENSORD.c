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
 *  ======== USBSENSORD.c ========
 */
/* Header files */
#include <ti/display/Display.h>

/* POSIX Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/ClockP.h>

/* driverlib Header files */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* usblib Header files */
#include <ti/usblib/msp432e4/usb-ids.h>
#include <ti/usblib/msp432e4/usblib.h>
#include <ti/usblib/msp432e4/usbhid.h>
#include <ti/usblib/msp432e4/device/usbdevice.h>
#include <ti/usblib/msp432e4/device/usbdhid.h>
#include <ti/usblib/msp432e4/device/usbdhidsensor.h>

/* Example/Board Header files */
#include "USBSENSORD.h"
#include "ti_usblib_config.h"

typedef uint32_t            USBSENSORDEventType;

/* Typedefs */
typedef volatile enum {
    USBSENSORD_STATE_IDLE = 0,
    USBSENSORD_STATE_SUSPENDED,
    USBSENSORD_STATE_SENDING,
    USBSENSORD_STATE_UNCONFIGURED
} USBSENSORD_USBState;

/* Static variables and handles */
static volatile USBSENSORD_USBState state;
Display_Handle displayHandle;

/* Function prototypes */
/*
 *  ======== USBSENSORD_hwiHandler ========
 *  Hardware interrupt handler for RTOS driver.
 *
 *  This functions calls the USB interrupt handler
 *
 *  @param(arg0)      Argument passed to the hardware handler
 *

 *  @return( )     None
 *
 */
static void USBSENSORD_hwiHandler(uintptr_t arg0);

/*
 *  ======== USB0_IRQDeviceHandler ========
 *  USB stack device interrupt handler
 *
 *  @param()       None
 *

 *  @return( )     None
 *
 */
extern void USB0_IRQDeviceHandler(void);

/*
 *  ======== USBSENSORD_hwiHandler ========
 *  Initializes the ADC inputs used by the sensor device.
 *  This example uses the ADC pins on Port E pins 1, 2,
 *  and 3(AIN0-2).
 *
 *  @param( )      None
 *

 *  @return( )     None
 *
 */
void ADCInit(void);

/*
 *  ======== cbSensorHandler ========
 */
USBSENSORDEventType cbSensorHandler(void *cbData, USBSENSORDEventType event,
                                             USBSENSORDEventType eventMsg, void *eventMsgPtr)
{
    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            state = USBSENSORD_STATE_IDLE;

            break;

        case USB_EVENT_DISCONNECTED:
            state = USBSENSORD_STATE_UNCONFIGURED;
            break;

        case USB_EVENT_TX_COMPLETE:
            state = USBSENSORD_STATE_IDLE;

            break;

        case USB_EVENT_SUSPEND:
            state = USBSENSORD_STATE_SUSPENDED;
            break;

        case USB_EVENT_RESUME:
            state = USBSENSORD_STATE_IDLE;

            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== USBSENSORD_hwiHandler ========
 */
static void USBSENSORD_hwiHandler(uintptr_t arg0)
{
    USB0_IRQDeviceHandler();
}

/*
 *  ======== USBSENSORD_init ========
 */
 void USBSENSORD_init(bool usbInternal)
 {
    HwiP_Handle hwi;
    uint32_t ui32ULPI;
    uint32_t ui32PLLRate;

    /* Install interrupt handler */
    hwi = HwiP_create(INT_USB0, USBSENSORD_hwiHandler, NULL);
    if (hwi == NULL)
    {
        //Can't create USB Hwi
        Display_printf(displayHandle, 0, 0, "Failed to create USB Hwi.\n");
        while(1);  
    }

    /* State specific variables */
    state = USBSENSORD_STATE_UNCONFIGURED;

    /* Check if the ULPI mode is to be used or not */
    if(!(usbInternal))
    {
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
    if (!USBDHIDSensorInit(0, &sensorDevice))
    {
        //Error initializing the Sensor
        Display_printf(displayHandle, 0, 0, "Error initializing the Sensor.\n");
       while(1);
    }
    /* Initialize the ADC channels.*/
    ADCInit();
}

 /*
  * ======== ADCInit ========
  */
void ADCInit(void)
{

    /*
     * Enable the clock to ADC-0 and wait for it to be ready
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /*
     *  Configure Sequencer 3 to sample a single analog channel : Internal
     * Temperature Sensor
     */
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_TS | ADC_CTL_IE |
                                 ADC_CTL_END);

    /*
     * Enable sample sequence 3 with a processor signal trigger.  Sequence 3
     * will do a single sample when the processor sends a signal to start the
     * conversion
     */
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 3);

    /*
     * Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample.
     */
    ADCIntClear(ADC0_BASE, 3);
}

/*
 *  ======== USBSENSORD_sendData ========
 */
int USBSENSORD_sendData(tSensorTemperatureReport *tempValueArray, unsigned int timeout)
{
    /* Indicate failure */

    switch (state) {
        case USBSENSORD_STATE_UNCONFIGURED:
             break;

        case USBSENSORD_STATE_SUSPENDED:
            state = USBSENSORD_STATE_SENDING;
            USBDHIDSensorSendReport(&sensorDevice, tempValueArray, sizeof(tempValueArray));
            break;

        case USBSENSORD_STATE_SENDING:
        case USBSENSORD_STATE_IDLE:
            /*Send the HID Report to host*/
            USBDHIDSensorSendReport(&sensorDevice, tempValueArray, sizeof(tempValueArray));
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== taskFxn ========
 */
void *sensorTaskFxn (void *arg0)
{
    tSensorTemperatureReport sReport;
    uint32_t getADCValue[1] = {0};
    uint32_t tempSenseinC;
    uint32_t time = 200000;


    bool *usbInternal = (bool *)arg0;

    Display_init();

    /* Create a Display */
    displayHandle = Display_open(Display_Type_UART, NULL);

    if (displayHandle == NULL) {
        /* Display_open() failed */
        while (1);
    }

    /*Initialize the sensor device*/
    USBSENSORD_init(usbInternal);

    while (true)
    {

        /* Trigger the ADC conversion. */
        ADCProcessorTrigger(ADC0_BASE, 3);

        /* Wait for conversion to be completed. */
        while(!ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        /* Clear the ADC interrupt flag. */
        ADCIntClear(ADC0_BASE, 3);

        /* Read ADC Value. */
        ADCSequenceDataGet(ADC0_BASE, 3, getADCValue);

        /* Convert raw ADC value to degree C and F */
        tempSenseinC = (1475*4096 - (75 * 33 * getADCValue[0]))/ 40960;

        /* store the temperature value as part of the input HID report
         * data.
         */
        sReport.i16Temp = (uint16_t)tempSenseinC;

        /* set the sensor state to ready as part of the input HID report data*/
        sReport.ui8SensorState = 1;  //sensor state is ready;

        /* set the sensor event to data updated as part of the input HID report data*/
        sReport.ui8SensorEvent = 19; //sensor event is data updated;

        USBSENSORD_sendData(&sReport, WAIT_FOREVER);

        /* Send data periodically */
        ClockP_usleep(time);
    }
}
