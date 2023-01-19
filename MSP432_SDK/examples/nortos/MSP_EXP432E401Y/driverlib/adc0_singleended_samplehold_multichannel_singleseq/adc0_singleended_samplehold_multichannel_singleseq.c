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
 * MSP432E4 Example project for ADC with multiple channel and single sequencer
 *
 * Description: In this application example the ADC0 is configured for a single
 * sequencer sampling 4 channels in single ended mode. Each channel is
 * configured for a different sample and hold duration. AIN0 is configured for
 * 32 clock cycle sample and hold, AIN1 is configured for 16 cycle sample and
 * hold, AIN2 is configured for 64 cycle sample and hold & AIN3 is configured
 * for 128 clock cycle sample and hold. The data is then displayed on the
 * serial console.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|               PE3|<-- AIN0
 *          | |               PE2|<-- AIN1
 *          --|RST            PE1|<-- AIN2
 *            |               PE0|<-- AIN3
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
    uint32_t getADCValue[4];
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

    /* Configure PE0-PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 2 to sample the analog channel : AIN0-AIN3. The
     * end of conversion and interrupt generation is set for AIN3. Each of
     * the channels is configured for different sample and hold duration. */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 |
                                 ADC_CTL_SHOLD_32);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH1 |
                                 ADC_CTL_SHOLD_16);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH2 |
                                 ADC_CTL_SHOLD_64);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_CH3 |
                                 ADC_CTL_SHOLD_128 | ADC_CTL_IE | ADC_CTL_END);

    /* Enable sample sequence 2 with a processor signal trigger.  Sequencer 2
     * will do a single sample when the processor sends a signal to start the
     * conversion */
    MAP_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 2);

    /* Since sample sequence 2 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 2);

    /* Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample. */
    MAP_ADCIntClear(ADC0_BASE, 2);

    /* Sample the sequencer forever.  Display the value on the console. */
    while(1)
    {
        /* Trigger the ADC conversion. */
        MAP_ADCProcessorTrigger(ADC0_BASE, 2);

        /* Wait for conversion to be completed. */
        while(!MAP_ADCIntStatus(ADC0_BASE, 2, false))
        {
        }

        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClear(ADC0_BASE, 2);

        /* Read ADC Value. */
        MAP_ADCSequenceDataGet(ADC0_BASE, 2, getADCValue);

        /* Display the AIN0-AIN03 (PE3-PE0) digital value on the console. */
        UARTprintf("AIN0-3 = %4d %4d %4d %4d\r", getADCValue[0],
                   getADCValue[1], getADCValue[2], getADCValue[3]);

        /* Delay the next sampling */
        MAP_SysCtlDelay(systemClock / 12);
    }
}
