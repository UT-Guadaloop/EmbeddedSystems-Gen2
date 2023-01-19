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
 * MSP432E4 Example Project for GPIO Per Pin Interrupt on Port P.
 *
 * Description: In this example, GPIO Port Pin P0-P3 are configured for edge
 * interrupt and correspondingly the LED D1-D4 are updated.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |               PP0|<--
 *          --|RST            PP1|<--
 *            |               PP2|<--
 *            |               PP3|<--
 *            |                  |
 *            |               PF0|-->LED D4
 *            |               PF4|-->LED D3
 *            |               PN0|-->LED D2
 *            |               PN1|-->LED D1
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

void GPIOP0_IRQHandler(void)
{
    uint32_t gpio_getPinP0State;

    /* Since it is per pin interrupt, clear the status register bit position */
    MAP_GPIOIntClear(GPIO_PORTP_BASE, GPIO_PIN_0);

    /* Read the state of the pin to decide if the pin is high or low and
     * accordingly set the LED ON or OFF */
    gpio_getPinP0State = MAP_GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_0);

    if(gpio_getPinP0State == GPIO_PIN_0)
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
    }
    else
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));
    }

}

void GPIOP1_IRQHandler(void)
{
    uint32_t gpio_getPinP1State;

    /* Since it is per pin interrupt, clear the status register bit position */
    MAP_GPIOIntClear(GPIO_PORTP_BASE, GPIO_PIN_1);

    /* Read the state of the pin to decide if the pin is high or low and
     * accordingly set the LED ON or OFF */
    gpio_getPinP1State = MAP_GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_1);

    if(gpio_getPinP1State == GPIO_PIN_1)
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
    }

}

void GPIOP2_IRQHandler(void)
{
    uint32_t gpio_getPinP2State;

    /* Since it is per pin interrupt, clear the status register bit position */
    MAP_GPIOIntClear(GPIO_PORTP_BASE, GPIO_PIN_2);

    /* Read the state of the pin to decide if the pin is high or low and
     * accordingly set the LED ON or OFF */
    gpio_getPinP2State = MAP_GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_2);

    if(gpio_getPinP2State == GPIO_PIN_2)
    {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
    else
    {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, ~(GPIO_PIN_4));
    }

}

void GPIOP3_IRQHandler(void)
{
    uint32_t gpio_getPinP3State;

    /* Since it is per pin interrupt, clear the status register bit position */
    MAP_GPIOIntClear(GPIO_PORTP_BASE, GPIO_PIN_3);

    /* Read the state of the pin to decide if the pin is high or low and
     * accordingly set the LED ON or OFF */
    gpio_getPinP3State = MAP_GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_3);

    if(gpio_getPinP3State == GPIO_PIN_3)
    {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else
    {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
    }

}

int main(void)
{
    /* Configure the system clock for 120 MHz */
    MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                            120000000);

    /* Enable the clock to the GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {

    }

    /* Configure the GPIO PN0-PN1 as output */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, (GPIO_PIN_0 | GPIO_PIN_1),
                     (GPIO_PIN_0 | GPIO_PIN_1));

    /* Enable the clock to the GPIO Port F and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)))
    {

    }

    /* Configure the GPIO PF0,PF4 as output */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, (GPIO_PIN_0 | GPIO_PIN_4));
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, (GPIO_PIN_0 | GPIO_PIN_4),
                     (GPIO_PIN_0 | GPIO_PIN_4));

    /* Enable the clock to the GPIO Port P and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP)))
    {

    }

    /* Configure the GPIO PP0-PP3 as input */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, (GPIO_PIN_2 | GPIO_PIN_3));

    /* Enable the Pull Up resistor on GPIO Port Pins PP0-PP3 */
    GPIOP->PUR = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;

    /* Configure the Per Pin Interrupt Enable */
    MAP_GPIOIntTypeSet(GPIO_PORTP_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
            GPIO_PIN_2 | GPIO_PIN_3), (GPIO_BOTH_EDGES | GPIO_DISCRETE_INT));
    MAP_GPIOIntEnable(GPIO_PORTP_BASE, (GPIO_INT_PIN_0 | GPIO_INT_PIN_1 |
            GPIO_INT_PIN_2 | GPIO_INT_PIN_3));
    MAP_IntEnable(INT_GPIOP0);
    MAP_IntEnable(INT_GPIOP1);
    MAP_IntEnable(INT_GPIOP2);
    MAP_IntEnable(INT_GPIOP3);

    while(1)
    {
        
    }
}
