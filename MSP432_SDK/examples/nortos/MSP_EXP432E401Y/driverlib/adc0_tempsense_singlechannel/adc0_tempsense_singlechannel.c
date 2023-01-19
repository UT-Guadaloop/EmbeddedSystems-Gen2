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
/******************************************************************************
 * MSP432E4 Example project for ADC with Temperature Sensor and single
 * sequencer
 *
 * Description: In this application example the ADC0 is configured for a single
 * sequencer sampling the internal temperature sensor. The data is then
 * displayed on the serial console.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

void ConfigureUART(uint32_t systemClock)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200 bps 8-N-1 format */
    UARTStdioConfig(0, 115200, systemClock);
}

int main(void)
{
    uint32_t getADCValue[1];
    uint32_t systemClock;
    uint32_t tempSenseinC;
    uint32_t tempSenseinF;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 3 to sample a single analog channel : Internal
     * Temperature Sensor */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_TS | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 3 with a processor signal trigger.  Sequence 3
     * will do a single sample when the processor sends a signal to start the
     * conversion */
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    /* Since sample sequence 3 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 3);

    /* Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample. */
    MAP_ADCIntClear(ADC0_BASE, 3);

    /* Sample Temperature Sensor forever.  Display the value on the console.*/
    while(1)
    {
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

        tempSenseinF = (tempSenseinC * 9)/5 + 32;

        /* Display the Temp Sense digital value on the console. */
        UARTprintf("\033[2J\033[H");
        UARTprintf("Raw Temp Sense = %4d\n", getADCValue[0]);
        UARTprintf("Temp Sense (C) = %3d\n", tempSenseinC);
        UARTprintf("Temp Sense (F) = %3d\n", tempSenseinF);

        /* Delay the next sampling */
        MAP_SysCtlDelay(systemClock / 12);
    }
}
