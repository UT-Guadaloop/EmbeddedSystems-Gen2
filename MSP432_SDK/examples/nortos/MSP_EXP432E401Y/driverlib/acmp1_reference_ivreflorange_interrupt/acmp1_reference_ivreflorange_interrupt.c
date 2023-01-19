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
 * MSP432E4 Example Project for Analog Comparator
 *
 * Description: In the application Analog Comparator 1 is configured to
 * generate an interrupt when the input signal is higher than the Internal
 * reference voltage from the Lo value ladder. At the same time the comparator
 * digital output also changes when the input crossed the trigger value. When
 * the input crosses the reference the LED D1 is lit. The User Switch SW2 can
 * be used to reset the state of the LED.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            C1-|<-- Input
 *            |               C1o|--> Output
 *            |                  |
 *            |               PJ1|<-- SW2
 *            |               PN1|--> LED
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

void GPIOJ_IRQHandler(void)
{
    uint32_t getSwitchState;

    /* Read the interrupt status register to find which switch button was
     * pressed and clear the interrupt status */
    getSwitchState = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    MAP_GPIOIntClear(GPIO_PORTJ_BASE, getSwitchState);

    /* If User Switch SW2 is pressed set the flag for read request */
    if(getSwitchState & GPIO_PIN_1)
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));
    }

}

void COMP1_IRQHandler(void)
{
    bool getIntStatus;

    /* Read the Interrupt status register */
    getIntStatus = MAP_ComparatorIntStatus(COMP_BASE, 1, true);

    if(getIntStatus)
    {
        /* If it is set then toggle the GPIO*/
        MAP_ComparatorIntClear(COMP_BASE, 1);

        /* Set the LED D1 ON */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
    }
}

int main(void)
{
    /* Configure the system clock for 120 MHz */
    MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                            120000000);

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ1 as input */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOJ->PUR = GPIO_PIN_1;

    /* Configure Interrupt Generation by Port Pin PJ0-PJ1 as falling edge */
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_1);
    MAP_IntEnable(INT_GPIOJ);

    /* Enable the clock to the GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN1 as output */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));

    /* Enable the clock to the GPIO Port C and D */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    /* Configure the GPIO for Analog Comparator */
    MAP_GPIOPinTypeComparator(GPIO_PORTC_BASE, GPIO_PIN_4);

    MAP_GPIOPinConfigure(GPIO_PD1_C1O);
    MAP_GPIOPinTypeComparatorOutput(GPIO_PORTD_BASE, GPIO_PIN_1);

    /* Enable the clock for Analog Comparator block */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_COMP0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_COMP0)))
    {
    }

    /* Configure the analog comparator to trigger an interrupt when the input
     * is more than internal Vref ladder in the hi-range. Also configure the
     * peripheral to make the output pin high when the condition is met */
    MAP_ComparatorConfigure(COMP_BASE, 1, (COMP_INT_RISE | COMP_ASRCP_REF |
                             COMP_OUTPUT_INVERT));
    MAP_ComparatorRefSet(COMP_BASE, COMP_REF_1_959375V);
    MAP_ComparatorIntEnable(COMP_BASE, 1);
    MAP_IntEnable(INT_COMP1);

    while(1)
    {
        
    }
}
