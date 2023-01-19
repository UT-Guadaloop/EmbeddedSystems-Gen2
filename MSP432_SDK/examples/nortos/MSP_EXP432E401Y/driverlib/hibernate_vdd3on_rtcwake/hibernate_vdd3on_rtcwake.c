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
 * MSP432E4 example code for Hibernate VDD3ON wake-up with RTC Wakeup.
 *
 * Description: The example puts the device in hibernate VDD3ON mode with wake
 * up from the RTC MATCH. The device in active state switched the LED D2 ON.
 * To put the device in hibernate the user must press the USR_SW1
 * which turns the LED D2 OFF and puts the device into hibernate state. The
 * device will automatically wakeup from Hibernate after 5 seconds. When the
 * wake from Hibernate is detected the LED D1 is switched ON. The status can
 * be cleared by pressing USR_SW2.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |               PJ1|<--USR_SW2
 *          --|RST            PJ0|<--USR_SW1
 *            |                  |
 *            |               PN1|-->LED D1
 *            |               PN0|-->LED D2
 *            |                  |
 *            |            WAKE_N|<--WAKE_SW4
 *            |                  |
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

volatile bool setHibEntry = false;

void HIBERNATE_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the Hibernate Interrupt Status*/
    getIntStatus = MAP_HibernateIntStatus(true);

    /* If wakeup is due to a RTC match then set the LED D1 ON */
    if(getIntStatus == HIBERNATE_INT_RTC_MATCH_0)
    {
        MAP_HibernateIntClear(HIBERNATE_INT_RTC_MATCH_0);

        /* Switch the LED D1 ON */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
    }
}

void GPIOJ_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the GPIO and clear the status */
    getIntStatus = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if((getIntStatus & GPIO_PIN_0) == GPIO_PIN_0)
    {
        MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

        /* Switch the LED D2 OFF */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

        setHibEntry = true;
    }

    if((getIntStatus & GPIO_PIN_1) == GPIO_PIN_1)
    {
        MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

        /* Switch the LED D1 OFF */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
    }
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
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN0-PN1 as output and switch it LED D2 ON */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, (GPIO_PIN_0 | GPIO_PIN_1), GPIO_PIN_0);

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0-PJ1 as input with internal pull up enabled.
     * Configure the PJ0-PJ1 for a falling edge interrupt detection */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    GPIOJ->PUR |= (GPIO_PIN_0 | GPIO_PIN_1);
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, (GPIO_PIN_0 | GPIO_PIN_1),
                                         GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, (GPIO_INT_PIN_0 | GPIO_INT_PIN_1));

    MAP_IntEnable(INT_GPIOJ);

    /* Enable the clock to the Hibernate and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE)))
    {
    }

    /* Configure Hibernate for VDD3ON Mode with Pin Wakeup if Hibernate
     * module has not been configured */
    if(!MAP_HibernateIsActive())
    {
        MAP_HibernateEnableExpClk(systemClock);
        MAP_HibernateWakeSet(HIBERNATE_WAKE_RTC);
        MAP_HibernateRTCSet(0);
        MAP_HibernateRTCEnable();
        HIB->CTL |= HIB_CTL_VDD3ON;
    }

    MAP_HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0);
    MAP_IntEnable(INT_HIBERNATE);

    /* Wait for the Hibernate entry flag to be set */
    while(!setHibEntry)
    {
    }

    /* Read the current RTC value and add 5 seconds to the same for
     * the wakeup interval */
    MAP_HibernateRTCMatchSet(0, (MAP_HibernateRTCGet()+5));
    MAP_HibernateRequest();

    while(1)
    {
    }
}
