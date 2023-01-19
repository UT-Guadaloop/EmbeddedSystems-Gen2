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
 * MSP432E4 Example project for Configuring a timer in 32-bit periodic mode
 *
 * Description: In this example, the timer is configured to generate an
 * interrupt every 1 second in 32-bit periodic mode. On the interrupt the state
 * of the LED is toggled.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |               PN0|-->LED
 *            |                  |
 *            |                  |
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

void TIMER0A_IRQHandler(void)
{
    uint32_t getTimerInterrupt;

    /* Get timer interrupt status  and clear the same */
    getTimerInterrupt = MAP_TimerIntStatus(TIMER0_BASE, true);
    MAP_TimerIntClear(TIMER0_BASE, getTimerInterrupt);

    /* Toggle the LED */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,
                     ~(MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0)));
}

int main(void)
{
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable the clock to the GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN0 as output and put in low state */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));

    /* Enable the Timer-0 in 32-bit periodic mode with interrupt generated
     * every 1 second */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, systemClock);
    MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* Enable Timer Interrupt */
    MAP_IntEnable(INT_TIMER0A);

    /* Start the timer */
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

    while(1)
    {
    }
}
