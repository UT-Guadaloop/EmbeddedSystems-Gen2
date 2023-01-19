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
 * MSP432E4 Example for Sleep Mode entry and wakeup using UART
 *
 * Description: The application code puts the device in sleep mode with clock
 * enabled only for the UART in sleep mode and memory in standby. During Active
 * state (RUN Mode), the LED D1 is switched ON. When there is no activity on
 * the terminal application the device is put into sleep mode and LED D1 is
 * switched OFF. When the user types a character in the terminal the device is
 * woken up and the LED D1 switches ON.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PA0|<-- UART RX
 *            |               PA1|--> UART TX
 *            |                  |
 *            |               PN1|--> LED D1
 *            |                  |
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

volatile uint32_t setTimeOutCount;
volatile uint32_t systemClock;


void UART0_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Set the timeout counter to count down value */
    setTimeOutCount = systemClock/12;

    /* Switch the LED ON */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);

    /* Get the interrupt status and clear the status bits */
    getIntStatus = MAP_UARTIntStatus(UART0_BASE, false);
    MAP_UARTIntClear(UART0_BASE, getIntStatus);

    /* Read the data and echo it to the terminal */
    MAP_UARTCharPut(UART0_BASE, MAP_UARTCharGet(UART0_BASE));
}

int main(void)
{
    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    setTimeOutCount = systemClock/12;

    /* Enable the clock to the GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN1 as output and switch D1 ON */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);

    /* Configure the GPIO for UART and the UART peripheral for 115200 bps 8-N-1*/
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, (GPIO_PIN_0 | GPIO_PIN_1));

    MAP_UARTConfigSetExpClk(UART0_BASE, systemClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    MAP_UARTIntEnable(UART0_BASE, UART_INT_RX);
    MAP_UARTEnable(UART0_BASE);
    MAP_UARTFIFODisable(UART0_BASE);

    MAP_IntEnable(INT_UART0);

    /* Configure GPIO Port N and UART0 to remain active in Sleep Mode */
    MAP_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);

    /* Configure the clocks to be auto gated in sleep mode */
    MAP_SysCtlPeripheralClockGating(true);

    /* Configure SRAM and Flash to be in Standby Power State */
    MAP_SysCtlSleepPowerSet(SYSCTL_SRAM_STANDBY | SYSCTL_FLASH_LOW_POWER);

    while(1)
    {
        /* Wait for any pending transfer from the UART peripheral */
        while(MAP_UARTBusy(UART0_BASE))
        {
        }

        /* Count down to 0 */
        for(;setTimeOutCount > 0; setTimeOutCount--);

        /* Switch the LED OFF and go to sleep state. */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));

        MAP_SysCtlSleep();
    }
}
