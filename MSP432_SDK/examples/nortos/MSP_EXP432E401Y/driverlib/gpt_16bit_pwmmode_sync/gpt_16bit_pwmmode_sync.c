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
 * MSP432E4 Example project for Configuring a multiple timer in 16-bit PWM
 * mode and sync.
 *
 * Description: In this example, the first timer is configured to generate a
 * PWM output with a frequency of 2 KHz and 66% duty cycle and another timer
 * with PWM output with a frequency of 2.001 kHz and 33% duty cycle. The
 * output for the two timers are out of sync. When the user presses the switch
 * SW2 the application synchronizes the output causing the PWM output to be
 * aligned. In between switch presses the two outputs shall drift and the
 * drift can be seen on a scope or a logic analyzer.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PL4|-->PWM (2.001 kHz 33%)
 *            |                  |
 *            |               PA7|-->PWM (2 kHz 66%)
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

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0-PJ1 as input */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOJ->PUR = GPIO_PIN_1;

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Configure the GPIO PA7 as Timer-3 CCP1 output */
    MAP_GPIOPinConfigure(GPIO_PA7_T3CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);

    /* Enable the clock to the GPIO Port L and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)))
    {
    }

    /* Configure the GPIO PA7 as Timer-0 CCP0 output */
    MAP_GPIOPinConfigure(GPIO_PL4_T0CCP0);
    MAP_GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4);

    /* Enable the Timer-3 in 16-bit PWM mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3)))
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

    /* Enable the Timer-0 in 16-bit PWM mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM);

    /* Set the Timer0A load value to systemClock/2001 for a 2.001 KHz output
     * PWM. To get a 33% duty cycle, configure the Match Value as 2/3 of the
     * load value. The timer shall count from load value to match value and
     * the output will be high. From the match value to the 0 the timer will
     * be low. */
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, systemClock/2001);

    MAP_TimerMatchSet(TIMER0_BASE, TIMER_A,
                  (MAP_TimerLoadGet(TIMER0_BASE, TIMER_A) * 2) / 3);

    /* Enable the timer count */
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

    while(1)
    {
        /* When the GPIO is Pressed and Released the timers are synchronized
         * */
        while(MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == GPIO_PIN_1);
        while(MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) != GPIO_PIN_1);

        MAP_SysCtlDelay(1000);

        MAP_TimerSynchronize(TIMER0_BASE, TIMER_0A_SYNC | TIMER_3B_SYNC);
    }
}
