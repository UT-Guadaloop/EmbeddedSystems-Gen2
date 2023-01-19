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
 * MSP432E4 Example project for Configuring a timer in 16-bit Edge Time mode
 *
 * Description: In this example, the timer is configured in Event Time mode.
 * The GPIO port J Pull Up is enabled so that a switch SW2 press causes an
 * event to be generated. The timer captures the falling edge and generates
 * an interrupt. The time stamp is captured and printed on the UART console.
 *
 *                MSP432E401Y       VDD
 *             ------------------    |
 *         /|\|                  |   /
 *          | |                  |   \
 *          --|RST               |   /
 *            |                  |   |----- SW2
 *            |               PA7|<--Event
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Standard Includes */
#include "uartstdio.h"

volatile uint16_t getTimerCaptureValue;
volatile bool bSetEventFlag;

void TIMER3B_IRQHandler(void)
{
    uint32_t getTimerIntStatus;

    /* Get the timer interrupt status and clear the same */
    getTimerIntStatus = MAP_TimerIntStatus(TIMER3_BASE, true);

    MAP_TimerIntClear(TIMER3_BASE, getTimerIntStatus);

    getTimerCaptureValue = MAP_TimerValueGet(TIMER3_BASE, TIMER_B);

    bSetEventFlag = 1;
}

void ConfigureUART(uint32_t systemClock)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200 bps 8-N-1 format */
    UARTStdioConfig(0, 115200, systemClock);
}

int main(void)
{
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Configure the UART for display on the Terminal */
    ConfigureUART(systemClock);

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0-PJ1 as input */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOJ->PUR = GPIO_PIN_1;

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Configure the GPIO PA7 as Timer-3 CCP1 pin */
    MAP_GPIOPinConfigure(GPIO_PA7_T3CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);

    /* Enable the Timer-3 in 16-bit Edge Time mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3)))
    {
    }

    /* Configure the Timer-3B in Edge Time Mode. Load the time to count with a
     * periodicity of 0.5 ms. */
    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME_UP);

    MAP_TimerControlEvent(TIMER3_BASE, TIMER_B, TIMER_EVENT_NEG_EDGE);

    MAP_TimerLoadSet(TIMER3_BASE, TIMER_B, systemClock/2000);

    MAP_TimerIntEnable(TIMER3_BASE, TIMER_CAPB_EVENT);

    MAP_TimerEnable(TIMER3_BASE, TIMER_B);

    /* Enable the timer interrupt */
    MAP_IntEnable(INT_TIMER3B);

    while(1)
    {
        /* Wait for an event capture and clear the flag */
        while(!(bSetEventFlag))
        {

        }

        bSetEventFlag = 0;

        UARTprintf("Event Captured at %d\n", getTimerCaptureValue);
    }
}
