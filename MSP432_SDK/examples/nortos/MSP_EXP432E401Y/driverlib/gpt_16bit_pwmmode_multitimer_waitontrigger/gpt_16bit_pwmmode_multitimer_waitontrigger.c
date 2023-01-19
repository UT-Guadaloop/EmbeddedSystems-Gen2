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
 * MSP432E4 Example project for Configuring multiple timers in 16-bit PWM
 * mode and wait on trigger.
 *
 * Description: In this example, all the timers are configured to generate PWM
 * output of 2 kHz and 66% duty cycle. However the timere are configured in
 * wait on trigger mode, such that when the first timer elapses it starts the
 * next timer in PWM mode. The output of each PWM can be monitored on a LA to
 * see how the timers trigger the next till all the timers generate a PWM out.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|               PL4|-->T0CCP0 (2 kHz 66%)
 *          | |               PL5|-->T0CCP1 (2 kHz 66%)
 *          --|RST            PL6|-->T1CCP0 (2 kHz 66%)
 *            |               PL7|-->T1CCP1 (2 kHz 66%)
 *            |               PM0|-->T2CCP0 (2 kHz 66%)
 *            |               PM1|-->T2CCP1 (2 kHz 66%)
 *            |               PM2|-->T3CCP0 (2 kHz 66%)
 *            |               PM3|-->T3CCP1 (2 kHz 66%)
 *            |               PM4|-->T4CCP0 (2 kHz 66%)
 *            |               PM5|-->T4CCP1 (2 kHz 66%)
 *            |               PM6|-->T5CCP0 (2 kHz 66%)
 *            |               PM7|-->T5CCP1 (2 kHz 66%)
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

    /* Enable the clock to the GPIO Port L and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)))
    {
    }

    /* Configure the GPIO PL4-PL7 as Timer-X CCPy output */
    MAP_GPIOPinConfigure(GPIO_PL4_T0CCP0);
    MAP_GPIOPinConfigure(GPIO_PL5_T0CCP1);
    MAP_GPIOPinConfigure(GPIO_PL6_T1CCP0);
    MAP_GPIOPinConfigure(GPIO_PL7_T1CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTL_BASE, (GPIO_PIN_4 | GPIO_PIN_5 |
                                           GPIO_PIN_6 | GPIO_PIN_7));

    /* Enable the clock to the GPIO Port M and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)))
    {
    }

    /* Configure the GPIO PM0-PM7 as Timer-X CCPy output */
    MAP_GPIOPinConfigure(GPIO_PM0_T2CCP0);
    MAP_GPIOPinConfigure(GPIO_PM1_T2CCP1);
    MAP_GPIOPinConfigure(GPIO_PM2_T3CCP0);
    MAP_GPIOPinConfigure(GPIO_PM3_T3CCP1);
    MAP_GPIOPinConfigure(GPIO_PM4_T4CCP0);
    MAP_GPIOPinConfigure(GPIO_PM5_T4CCP1);
    MAP_GPIOPinConfigure(GPIO_PM6_T5CCP0);
    MAP_GPIOPinConfigure(GPIO_PM7_T5CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTM_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
                                           GPIO_PIN_2 | GPIO_PIN_3 |
                                           GPIO_PIN_4 | GPIO_PIN_5 |
                                           GPIO_PIN_6 | GPIO_PIN_7));

    /* Enable the Timers 0-5 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);

    /* Configure each of the timer in 16-bit PWM mode with 2 KHz 66% duty
     * cycle. */
    MAP_TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM |
                                    TIMER_CFG_B_PWM));
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_BOTH, systemClock/2000);
    MAP_TimerMatchSet(TIMER0_BASE, TIMER_BOTH,
                  MAP_TimerLoadGet(TIMER0_BASE, TIMER_A) / 3);

    MAP_TimerConfigure(TIMER1_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM |
                                    TIMER_CFG_B_PWM));
    MAP_TimerLoadSet(TIMER1_BASE, TIMER_BOTH, systemClock/2000);
    MAP_TimerMatchSet(TIMER1_BASE, TIMER_BOTH,
                  MAP_TimerLoadGet(TIMER1_BASE, TIMER_A) / 3);

    MAP_TimerConfigure(TIMER2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM |
                                    TIMER_CFG_B_PWM));
    MAP_TimerLoadSet(TIMER2_BASE, TIMER_BOTH, systemClock/2000);
    MAP_TimerMatchSet(TIMER2_BASE, TIMER_BOTH,
                  MAP_TimerLoadGet(TIMER2_BASE, TIMER_A) / 3);

    MAP_TimerConfigure(TIMER3_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM |
                                    TIMER_CFG_B_PWM));
    MAP_TimerLoadSet(TIMER3_BASE, TIMER_BOTH, systemClock/2000);
    MAP_TimerMatchSet(TIMER3_BASE, TIMER_BOTH,
                  MAP_TimerLoadGet(TIMER3_BASE, TIMER_A) / 3);

    MAP_TimerConfigure(TIMER4_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM |
                                    TIMER_CFG_B_PWM));
    MAP_TimerLoadSet(TIMER4_BASE, TIMER_BOTH, systemClock/2000);
    MAP_TimerMatchSet(TIMER4_BASE, TIMER_BOTH,
                  MAP_TimerLoadGet(TIMER4_BASE, TIMER_A) / 3);

    MAP_TimerConfigure(TIMER5_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM |
                                    TIMER_CFG_B_PWM));
    MAP_TimerLoadSet(TIMER5_BASE, TIMER_BOTH, systemClock/2000);
    MAP_TimerMatchSet(TIMER5_BASE, TIMER_BOTH,
                  MAP_TimerLoadGet(TIMER5_BASE, TIMER_A) / 3);

    /* Only Timer-0 Sub timer-A will not have a wait on trigger */
    MAP_TimerControlWaitOnTrigger(TIMER0_BASE, TIMER_A, false);
    MAP_TimerControlWaitOnTrigger(TIMER0_BASE, TIMER_B, true);
    MAP_TimerControlWaitOnTrigger(TIMER1_BASE, TIMER_BOTH, true);
    MAP_TimerControlWaitOnTrigger(TIMER2_BASE, TIMER_BOTH, true);
    MAP_TimerControlWaitOnTrigger(TIMER3_BASE, TIMER_BOTH, true);
    MAP_TimerControlWaitOnTrigger(TIMER4_BASE, TIMER_BOTH, true);
    MAP_TimerControlWaitOnTrigger(TIMER5_BASE, TIMER_BOTH, true);

    /* Enable the timer-0 sub-timer-A count */
    MAP_TimerEnable(TIMER5_BASE, TIMER_BOTH);
    MAP_TimerEnable(TIMER4_BASE, TIMER_BOTH);
    MAP_TimerEnable(TIMER3_BASE, TIMER_BOTH);
    MAP_TimerEnable(TIMER2_BASE, TIMER_BOTH);
    MAP_TimerEnable(TIMER1_BASE, TIMER_BOTH);
    MAP_TimerEnable(TIMER0_BASE, TIMER_BOTH);

    while(1)
    {
    }
}
