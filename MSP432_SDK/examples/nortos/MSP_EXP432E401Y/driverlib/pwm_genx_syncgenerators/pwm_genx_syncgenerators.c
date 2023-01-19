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
 * MSP432E4 PWM Generation with synchronization
 *
 * Description: Several PWMs are generated with local synchronization.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |       PF1(M0PWM1)|---> 25% PWM
 *            |       PF2(M0PWM2)|---> 50% PWM
 *            |       PF3(M0PWM3)|---> 75% PWM
 *            |                  |
 *            |       PG0(M0PWM4)|---> 25% PWM
 *            |       PG1(M0PWM5)|---> 75% PWM (deadband pair)
 *            |                  |
 *            |       PK4(M0PWM6)|---> 75% PWM
 *            |       PK5(M0PWM7)|---> 25% PWM (deadband pair)
 *            |                  |
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
    UARTprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    UARTprintf("                                        ");
    UARTprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    MAP_SysCtlDelay(getSystemClock / 3);
}

/* Main function loop */
int main(void)
{
    volatile bool bsyncMode;
    volatile uint16_t setDelay;

    /* Configure the system clock for 16 MHz internal oscillator */
    getSystemClock = MAP_SysCtlClockFreqSet((SYSCTL_OSC_INT |
                                             SYSCTL_USE_OSC), 16000000);

    /* Initialize serial console */
    InitConsole();

    /* Display the setup on the console. */
    UARTprintf("PWM ->\n");
    UARTprintf("  Module: PWM0\n");
    UARTprintf("  Pin(s): PG0 and PG1, PK4 and PK5\n");
    UARTprintf("  Features: Synchronized Dead-band Generation\n");
    UARTprintf("  Duty Cycle: 25%% on PD0 and 75%% on PD1\n");
    UARTprintf("  Dead-band Length: 160 cycles on rising and falling edges\n\n");
    UARTprintf("Generating PWM on PWM0 (PF1,PF2,PF3) \n ");

    /* The PWM peripheral must be enabled for use. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)));

    /* Set the PWM clock to the system clock. */
    MAP_PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_1);

    /* Enable the clock to the GPIO Port F, G and K for PWM pins */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));

    /* Generator 0 Pins */
    MAP_GPIOPinConfigure(GPIO_PF1_M0PWM1);

    /* Generator 1 Pins */
    MAP_GPIOPinConfigure(GPIO_PF2_M0PWM2);
    MAP_GPIOPinConfigure(GPIO_PF3_M0PWM3);

    /* Generator 2 Pins */
    MAP_GPIOPinConfigure(GPIO_PG0_M0PWM4);
    MAP_GPIOPinConfigure(GPIO_PG1_M0PWM5);

    /* Generator 3 Pins */
    MAP_GPIOPinConfigure(GPIO_PK4_M0PWM6);
    MAP_GPIOPinConfigure(GPIO_PK5_M0PWM7);

    MAP_GPIOPinTypePWM(GPIO_PORTF_BASE, (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3));
    MAP_GPIOPinTypePWM(GPIO_PORTG_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    MAP_GPIOPinTypePWM(GPIO_PORTK_BASE, (GPIO_PIN_4 | GPIO_PIN_5));

    /* Configure the PWM0 to count up/down without synchronization.
     * Note: Enabling the dead-band generator automatically couples the 2
     * outputs from the PWM block so we don't use the PWM synchronization. */
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN |
                        PWM_GEN_MODE_GEN_SYNC_LOCAL);
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN |
                        PWM_GEN_MODE_GEN_SYNC_LOCAL);

    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN |
                        PWM_GEN_MODE_DB_SYNC_LOCAL);
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN |
                        PWM_GEN_MODE_DB_SYNC_LOCAL);

    /* Set the PWM period to 250Hz.  To calculate the appropriate parameter
     * use the following equation: N = (1 / f) * SysClk.  Where N is the
     * function parameter, f is the desired frequency, and SysClk is the
     * system clock frequency.
     * In this case you get: (1 / 250Hz) * 16MHz = 64000 cycles.  Note that
     * the maximum period you can set is 2^16 - 1.
     * TODO: modify this calculation to use the clock frequency that you are
     * using. */
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 64000);
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 64000);
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, 64000);
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, 64000);

    /* PWMs           Positive Duty Cycle
     * Bit 1 (PF1)    25%
     * Bit 2 (PF2)    50%
     * Bit 3 (PF3)    75%
     * Bit 4 (PG0)    25%
     * Bit 5 (PG1)    75% (dead band from Bit4)
     * Bit 6 (PK4)    75%
     * Bit 7 (PK5)    25% (dead band from Bit6) */
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,
                     MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 4);
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,
                     MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 2);
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,
                     3*MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 4);

    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,
                     MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2) / 4);
    MAP_PWMDeadBandEnable(PWM0_BASE, PWM_GEN_2, 160, 160);

    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,
                     3*MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_3) / 4);
    MAP_PWMDeadBandEnable(PWM0_BASE, PWM_GEN_3, 160, 160);

    /* Enable the PWM0 output signals
     *   Bit 1 (PF1) Bit 2 (PF2) and Bit 3 (PF3)
     *   Bit 4 (PG0) and Bit 5 (PG1)
     *   Bit 6 (PK4) and Bit 7 (PK5) */
    MAP_PWMOutputState(PWM0_BASE, ( PWM_OUT_1_BIT | PWM_OUT_2_BIT | PWM_OUT_3_BIT |
                                     PWM_OUT_4_BIT | PWM_OUT_5_BIT |
                                     PWM_OUT_6_BIT | PWM_OUT_7_BIT ) , true);

    MAP_PWMOutputInvert(PWM0_BASE, (PWM_OUT_6_BIT | PWM_OUT_7_BIT) , true);

    /* Enables the counter for a PWM generator block. */
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_2);

    for(setDelay = 0; setDelay < 10000; setDelay++);

    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_1);
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_3);

    MAP_PWMSyncTimeBase(PWM0_BASE, (PWM_GEN_2_BIT | PWM_GEN_3_BIT));

    /* Loop forever while the PWM signals are generated. */
    while(1)
    {
        if(bsyncMode == true)
        {
            MAP_PWMOutputUpdateMode(PWM0_BASE,
                                    ( PWM_OUT_1_BIT | PWM_OUT_2_BIT | PWM_OUT_3_BIT),
                                    PWM_OUTPUT_MODE_NO_SYNC
                                    );

            /* Reset Generator 0 counter to 0 so out of sync with Generator 1 */
            MAP_PWMSyncTimeBase(PWM0_BASE, PWM_GEN_0_BIT);
            bsyncMode = false;

            UARTprintf(" PF1 not in Sync with PF2/3 ->");
        }
        else
        {
            MAP_PWMOutputUpdateMode(PWM0_BASE,
                                    ( PWM_OUT_1_BIT | PWM_OUT_2_BIT | PWM_OUT_3_BIT),
                                    PWM_OUTPUT_MODE_SYNC_LOCAL
                                    );

            /* Reset both generator 0 and 1 counters to 0 to sync */
            MAP_PWMSyncTimeBase(PWM0_BASE, (PWM_GEN_0_BIT | PWM_GEN_1_BIT));
            bsyncMode = true;

            UARTprintf(" PF1 in Sync with PF2/3 ->");
        }

        /* Print out indication on the console that the program is running. */
        PrintRunningDots();
    }
}
