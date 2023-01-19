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
 * MSP432E4 Example project for ADC use for Digital Comparator
 *
 * Description: In this application example the ADC0 is configured for a single
 * sequencer sampling a single channel in single ended mode. The sequencer is
 * triggered using a Timer and the data from the channel is sent to the
 * Digital Comparator block. The Digital Comparator is configured to generate
 * an interrupt once when the data is below the programmed low band. When the
 * interrupt is triggered the LED D2 is toggled.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PE3|<-- AIN0
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PN0|--> LED D2
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

void ADC0SS3_IRQHandler(void)
{
    uint32_t getIntStatus;
    uint32_t getDCompStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* If the interrupt status for Sequencer-3 is set the
     * clear the status and read the data */
    if(getIntStatus == ADC_INT_DCON_SS3)
    {
        /* Get the Comparator Interrupt status and clear the same */
        getDCompStatus = MAP_ADCComparatorIntStatus(ADC0_BASE);
        MAP_ADCComparatorIntClear(ADC0_BASE, getDCompStatus);

        /* Clear the interrupt condition */
        MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DCON_SS3);

        /* Toggle the LED ON */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,
                         ~MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0));
    }
}

int main(void)
{
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable the clock to GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure PN0 as Output for controlling the LED */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));

    /* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }

    /* Configure PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 3 to sample a single analog channel : AIN0 and send
     * the channel data to the Digital Comparator */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_CMP1 |
                                 ADC_CTL_END);

    /* Enable sample sequence 3 with a timer trigger.  Sequence 3 will do a
     * single sample when the timer sends a signal to start the conversion */
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 3);

    /* Configure the comparator to have low threshold of VREF/3 and a high
     * threshold of 2*VREF/3. The VREF is 3.3V and has output of 4095. This
     * results in a low threshold of 0x555 and high threshold of 0xAAA */
    MAP_ADCComparatorConfigure(ADC0_BASE, 1, ADC_COMP_INT_LOW_ONCE);
    MAP_ADCComparatorRegionSet(ADC0_BASE, 1, 0x555, 0xAAA);
    MAP_ADCComparatorReset(ADC0_BASE, 1, false, true);
    MAP_ADCComparatorIntEnable(ADC0_BASE, 3);

    /* Since sample sequence 3 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 3);

    /* Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample. */
    MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DCON_SS3);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_DCON_SS3);

    /* Enable the Interrupt generation from the ADC-0 Sequencer 3*/
    MAP_IntEnable(INT_ADC0SS3);

    /* Enable Timer-0 clock and configure the timer in periodic mode with
     * a frequency of 1 KHz. Enable the ADC trigger generation from the
     * timer-0. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (systemClock/1000));
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

    /* Empty Loop */
    while(1)
    {
    }
}
