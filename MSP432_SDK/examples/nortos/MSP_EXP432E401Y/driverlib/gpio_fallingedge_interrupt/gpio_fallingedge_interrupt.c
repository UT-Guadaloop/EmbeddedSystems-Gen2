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
 * MSP432E4 Example Project to demonstrate GPIO falling edge interrupt
 *
 * Description: In this example the GPIO Port J Pin-0 is configured to generate
 * an interrupt on falling edge detect. When the user presses the USR_SW1 the
 * GPIO for LED D2 is toggled.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PN0|--> LED D2
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PJ0|<-- USR_SW1
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
    uint32_t getIntStatus;

    /* Get the interrupt status from the GPIO and clear the status */
    getIntStatus = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if((getIntStatus & GPIO_PIN_0) == GPIO_PIN_0)
    {
        MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

        /* Toggle the LED */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,
                         ~(MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0)));
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
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN0 as output */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0 as input with internal pull up enabled. Configure
     * the PJ0 for a falling edge interrupt detection */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR |= GPIO_PIN_0;
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);

    MAP_IntEnable(INT_GPIOJ);

    while(1)
    {
        
    }
}
