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
 * MSP432E4 Example project for ADC with multiple channel and multiple
 * sequencers
 *
 * Description: In this application example the ADC0 is configured for multiple
 * sequencers sampling 7 channels in single ended mode. The data is then
 * displayed on the serial console.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|               PE3|<-- AIN0
 *          | |               PE2|<-- AIN1
 *          --|RST            PE1|<-- AIN2
 *            |               PE0|<-- AIN3
 *            |               PD7|<-- AIN4
 *            |               PD6|<-- AIN5
 *            |               PD5|<-- AIN6
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
    uint32_t getADCValue[7];
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }

    /* Enable the clock to GPIO Port D and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)))
    {
    }

    /* Configure PE0-PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
    MAP_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    MAP_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_6);
    MAP_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 0 to sample the analog channel : AIN0-AIN2. The
     * end of conversion and interrupt generation is set for AIN2 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Configure Sequencer 1 to sample the analog channel : AIN3-AIN4. The
     * end of conversion and interrupt generation is set for AIN4 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH3);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH4 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Configure Sequencer 2 to sample the analog channel : AIN5-AIN6. The
     * end of conversion and interrupt generation is set for AIN6 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH5);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH6 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 0-2 with a processor signal trigger.  Sequencers
     * will do a single sample when the processor sends a signal to start the
     * conversion. Also make sure that the priority is set with SS0 being the
     * highest and SS2 being the lowest */
    MAP_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    MAP_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 1);
    MAP_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 2);

    /* Since sample sequence 0-2 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 0);
    MAP_ADCSequenceEnable(ADC0_BASE, 1);
    MAP_ADCSequenceEnable(ADC0_BASE, 2);

    /* Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample. */
    MAP_ADCIntClear(ADC0_BASE, 0);
    MAP_ADCIntClear(ADC0_BASE, 1);
    MAP_ADCIntClear(ADC0_BASE, 2);

    /* Sample AIN0 forever.  Display the value on the console. */
    while(1)
    {
        /* Trigger the ADC conversion for all sequencers simultaneously.
         * The ADC shall take care of performing conversion based on the
         * priority assigned to each sequencer. */
        ADC0->PSSI = 0x7;

        /* Wait for conversion to be completed for Sequencer 2 as it has
         * the lowest priority. If the priority changes then change the
         * sequencer number which has the lowest priority. */
        while(!MAP_ADCIntStatus(ADC0_BASE, 2, false))
        {
        }

        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClear(ADC0_BASE, 2);

        /* Read ADC Value. */
        MAP_ADCSequenceDataGet(ADC0_BASE, 0, &getADCValue[0]);
        MAP_ADCSequenceDataGet(ADC0_BASE, 1, &getADCValue[3]);
        MAP_ADCSequenceDataGet(ADC0_BASE, 2, &getADCValue[5]);

        /* Display the AIN0-AIN06 digital value on the console. */
        UARTprintf("\033[2J\033[H");
        UARTprintf("Sequencer-0 AIN0-2 = %4d %4d %4d\n", getADCValue[0],
                   getADCValue[1], getADCValue[2]);
        UARTprintf("Sequencer-1 AIN3-4 = %4d %4d\n", getADCValue[3],
                   getADCValue[4]);
        UARTprintf("Sequencer-2 AIN5-6 = %4d %4d\n", getADCValue[5],
                   getADCValue[6]);

        /* Delay the next sampling */
        MAP_SysCtlDelay(systemClock / 3);
    }
}
