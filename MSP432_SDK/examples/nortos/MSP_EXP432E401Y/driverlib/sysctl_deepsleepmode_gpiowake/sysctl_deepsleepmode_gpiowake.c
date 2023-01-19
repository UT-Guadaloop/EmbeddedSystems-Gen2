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
 * MSP432E4 Example for Deep Sleep Mode entry and wakeup using GPIO
 *
 * Description: The application code puts the device in deep sleep mode with
 * clock enabled only for the GPIO in deep sleep mode and memory in standby.
 * During Active state (RUN Mode), the LED D1 is switched ON. When there is no
 * activity the device is put into deep sleep mode and LED D1 is switched OFF.
 * During Deep sleep the clock source if LFIOSC, the core voltage is reduced
 * to 0.9V and memory banks are put in low power state. When the user presses
 * USR_SW2 the device is woken up and the LED D1 switches ON.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PJ1|<-- USR_SW2
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


void GPIOJ_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Set the timeout counter to count down value */
    setTimeOutCount = systemClock/12;

    /* Get the interrupt status and clear the status bits */
    getIntStatus = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, false);
    MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

    /* Switch the LED ON */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
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

    /* Configure the GPIO PJ1 as input and enable interrupt on rising edge */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOJ->PUR |= GPIO_PIN_1;

    /* Configure Interrupt Generation by Port Pin PJ1 as falling edge */
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_1);
    MAP_IntEnable(INT_GPIOJ);

    /* Configure GPIO Port N and J to remain active in Deep Sleep Mode */
    MAP_SysCtlPeripheralDeepSleepEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralDeepSleepEnable(SYSCTL_PERIPH_GPIOJ);

    /* Configure the clocks to be auto gated in deep sleep mode */
    MAP_SysCtlPeripheralClockGating(true);

    /* Configure SRAM and Flash to be in Low Power State */
    MAP_SysCtlDeepSleepPowerSet(SYSCTL_SRAM_LOW_POWER | SYSCTL_FLASH_LOW_POWER);

    /* Configure the deep sleep clock as LFIOSC and power down the PIOSC */
    MAP_SysCtlDeepSleepClockConfigSet((SYSCTL_DSLP_OSC_INT30 |
            SYSCTL_DSLP_PIOSC_PD | SYSCTL_DSLP_MOSC_DPD), 1);

    /* Configure the VDDC voltage to 0.9V. On the LaunchPad the user can check
     * TP12 for change of VDDC from 1.2V to 0.9V */
    MAP_SysCtlLDODeepSleepSet(SYSCTL_LDO_0_90V);

    while(1)
    {
        /* Count down to 0 */
        for(;setTimeOutCount > 0; setTimeOutCount--);

        /* Switch the LED OFF and go to deep sleep state. */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));

        MAP_SysCtlDeepSleep();
    }
}
