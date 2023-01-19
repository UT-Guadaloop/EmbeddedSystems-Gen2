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
 * MSP432E4 PWM with deadband interrupt Project
 *
 * Description: This project creates two PWMs from PWM Generator 3.  Each PWM
 *              period is 128K clock cycles(64K up and 64K down); ~250Hz.  PWMA
 *              has a 25% positive duty cycle while PWMB has 75%.  The PWM
 *              interrupt sources for Generator 1 are the PWMA comparator value
 *              in both the up and down directions and the Load and Zero count
 *              values.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |               PA0|<--- UART RX
 *            |               PA1|---> UART TX
 *            |                  |
 *            |               PF2|---> PWMA 25% Duty Cycle
 *            |               PF3|---> PWMB 75% Duty Cycle
 *            |                  |
 *
 * Author: C. Sterzik
 *
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Include for serial console */
#include "uartstdio.h"

/* Global variable for system clock */
uint32_t getSystemClock;

/* PWM ISR */
void PWM0_1_IRQHandler(void)
{
    uint32_t getIntStatus;

    getIntStatus = MAP_PWMGenIntStatus(PWM0_BASE, PWM_GEN_1, true);

    MAP_PWMGenIntClear(PWM0_BASE, PWM_GEN_1, getIntStatus);

}

/* Function to setup the serial console */
void InitConsole(void)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200 bps 8-N-1 format with internal 16 MHz
     * oscillator as the UART clock source */
    MAP_UARTClockSourceSet(UART0_BASE, UART_CLOCK_ALTCLK);

    UARTStdioConfig(0, 115200, 16000000);
}

/* Prints out 5x "." with a second delay after each print.  This function will
 * then backspace, clearing the previously printed dots, and then backspace
 * again so you can continuously printout on the same line.  The purpose of
 * this function is to indicate to the user that the program is running. */
void
PrintRunningDots(void)
{
    UARTprintf(". ");

    MAP_SysCtlDelay(getSystemClock / 3);
    UARTprintf(". ");
    MAP_SysCtlDelay(getSystemClock / 3);
    UARTprintf(". ");
    MAP_SysCtlDelay(getSystemClock / 3);

    UARTprintf(". ");
    MAP_SysCtlDelay(getSystemClock / 3);
    UARTprintf(". ");
    MAP_SysCtlDelay(getSystemClock / 3);
    UARTprintf("\b\b\b\b\b\b\b\b\b\b");
    UARTprintf("          ");
    UARTprintf("\b\b\b\b\b\b\b\b\b\b");
    MAP_SysCtlDelay(getSystemClock / 3);
}

int main(void)
{
    /* Configure the system clock for 16 MHz internal oscillator */
    getSystemClock = MAP_SysCtlClockFreqSet((SYSCTL_OSC_INT |
                                             SYSCTL_USE_OSC), 16000000);

    /* Initialize serial console */
    InitConsole();

    /* Display the setup on the console. */
    UARTprintf("PWM ->\n");
    UARTprintf("  Module: PWM0 Generator 1\n");
    UARTprintf("  Pin(s): PF2 and PF3\n");
    UARTprintf("  Features: Dead-band Generation\n");
    UARTprintf("  Duty Cycle: 25%% on PF2 and 75%% on PF3\n");
    UARTprintf("  Dead-band Length: 160 cycles on rising and falling edges\n\n");
    UARTprintf("Generating PWM on PWM0 (PF2/PF3) -> ");

    /* The PWM peripheral must be enabled for use. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)));

    /* Set the PWM clock to the system clock. */
    MAP_PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_1);

    /* Enable the clock to the GPIO Port F for PWM pins */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    MAP_GPIOPinConfigure(GPIO_PF2_M0PWM2);
    MAP_GPIOPinConfigure(GPIO_PF3_M0PWM3);
    MAP_GPIOPinTypePWM(GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3));

    /* Configure the PWM0 to count up/down without synchronization.
     * Note: Enabling the dead-band generator automatically couples the 2
     * outputs from the PWM block so we don't use the PWM synchronization. */
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN |
                        PWM_GEN_MODE_NO_SYNC);

    /* Set the PWM period to 250Hz.  To calculate the appropriate parameter
     * use the following equation: N = (1 / f) * SysClk.  Where N is the
     * function parameter, f is the desired frequency, and SysClk is the
     * system clock frequency.
     * In this case you get: (1 / 250Hz) * 16MHz = 64000 cycles.  Note that
     * the maximum period you can set is 2^16 - 1. */
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 64000);

    /* Set PWM0 PK4 to a duty cycle of 25%.  You set the duty cycle as a
     * function of the period.  Since the period was set above, you can use the
     * PWMGenPeriodGet() function.  For this example the PWM will be high for
     * 25% of the time or 16000 clock cycles (64000 / 4). */
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,
                     MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 4);

    /* Enable the dead-band generation on the PWM0 output signal.  PWM bit 2
     * (PF2), will have a duty cycle of 25% (set above) and PWM bit 3 will have
     * a duty cycle of 75%.  These signals will have a 10us gap between the
     * rising and falling edges.  This means that before PWM bit 2 goes high,
     * PWM bit 3 has been low for at LEAST 160 cycles (or 10us) and the same
     * before PWM bit 3 goes high.  The dead-band generator lets you specify
     * the width of the "dead-band" delay, in PWM clock cycles, before the PWM
     * signal goes high and after the PWM signal falls.  For this example we
     * will use 160 cycles (or 10us) on both the rising and falling edges of
     * PF2.  Reference the datasheet for more information on dead-band
     * generation. */
    MAP_PWMDeadBandEnable(PWM0_BASE, PWM_GEN_1, 160, 160);

    MAP_IntMasterEnable();

    /* This timer is in up-down mode.  Interrupts will occur when the
     * counter for this PWM counts to the load value (64000), when the
     * counter counts up to 64000/4 (PWM A Up), counts down to 64000/4
     * (PWM A Down), and counts to 0. */
    MAP_PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_1,
                            PWM_INT_CNT_ZERO | PWM_INT_CNT_LOAD |
                            PWM_INT_CNT_AU | PWM_INT_CNT_AD);
    MAP_IntEnable(INT_PWM0_1);
    MAP_PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);

    /* Enable the PWM0 Bit 2 (PF2) and Bit 3 (PF3) output signals. */
    MAP_PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT | PWM_OUT_3_BIT, true);

    /* Enables the counter for a PWM generator block. */
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_1);

    /* Loop forever while the PWM signals are generated. */
    while(1)
    {
        /* Print out indication on the console that the program is running. */
        PrintRunningDots();
    }
}
