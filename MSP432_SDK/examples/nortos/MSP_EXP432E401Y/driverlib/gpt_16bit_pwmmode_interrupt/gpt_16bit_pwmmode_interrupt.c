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
 * MSP432E4 Example project for Configuring a timer in 16-bit PWM mode
 *
 * Description: In this example, the timer is configured to generate a PWM
 * output with a frequency of 2 KHz and 66% duty cycle
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |               PA7|-->PWM
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

int main(void)
{
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Configure the GPIO PA7 as Timer-3 CCP1 output */
    MAP_GPIOPinConfigure(GPIO_PA7_T3CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);

    /* Enable the Timer-3 in 16-bit PWM mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3)))
    {
    }

    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);

    /* Set the Timer3B load value to systemClock/2000 for a 2 KHz output PWM
     * To get a 66% duty cycle, configure the Match Value as 1/3 of the load
     * value. The timer shall count from load value to match value and the
     * output will be high. From the match value to the 0 the timer will be
     * low. */
    MAP_TimerLoadSet(TIMER3_BASE, TIMER_B, systemClock/2000);

    MAP_TimerMatchSet(TIMER3_BASE, TIMER_B,
                  MAP_TimerLoadGet(TIMER3_BASE, TIMER_B) / 3);

    /* Enable the timer count */
    MAP_TimerEnable(TIMER3_BASE, TIMER_B);

    while(1)
    {
    }
}
