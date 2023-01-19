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
 * MSP432E4 example code for Hibernate VDD3ON wake-up with Tamper Detection.
 *
 * Description: The example puts the device in hibernate VDD3ON mode with wake
 * up due to Tamper Detection. The Tamper pins are configured to detect a low
 * level tamper event and Crystal failure is also enabled. The devcie enters
 * hibernate when user presses the switch SW1. When a Tamper event occurs the
 * NMI is called.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PJ0|<--USR_SW1
 *            |                  |
 *            |                  |
 *            |               PN0|-->LED D2
 *            |                  |
 *            |            WAKE_N|
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

void NMI_Handler(void)
{
    uint32_t getRTCValue;
    uint32_t getTamperValue;

    /* Get the cause of the Tamper Event */
    MAP_HibernateTamperEventsGet(0, &getRTCValue, &getTamperValue);

    MAP_HibernateTamperEventsClear();

    /* Switch the LED D1 ON */
    if(getTamperValue == HIBERNATE_TAMPER_EVENT_0)
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, (GPIO_PIN_0 | GPIO_PIN_1),
                                          (GPIO_PIN_0 | GPIO_PIN_1));
    }
    else
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }

    setHibEntry = false;
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
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, (GPIO_PIN_0 | GPIO_PIN_1), 0);

        setHibEntry = true;
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

    /* Configure the GPIO PJ0 as input with internal pull up enabled.
     * Configure the PJ0 for a falling edge interrupt detection */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR |= GPIO_PIN_0;
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);

    MAP_IntEnable(INT_GPIOJ);

    /* Enable the clock to the Hibernate and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE)))
    {
    }

    /* Configure Hibernate for VDD3ON Mode if Hibernate module has not been
     * configured. Enable the Tamper Pin-0 for tamper detection */
    if(!MAP_HibernateIsActive())
    {
        MAP_HibernateEnableExpClk(systemClock);
        MAP_HibernateRTCSet(0);
        MAP_HibernateRTCEnable();
        MAP_HibernateWakeSet(HIBERNATE_WAKE_PIN);
        HIB->CTL |= HIB_CTL_VDD3ON;
        MAP_HibernateTamperIOEnable(0, HIBERNATE_TAMPER_IO_MATCH_SHORT |
                                       HIBERNATE_TAMPER_IO_WPU_ENABLED |
                                       HIBERNATE_TAMPER_IO_TRIGGER_LOW);
        MAP_HibernateTamperEventsConfig(HIBERNATE_TAMPER_EVENTS_HIB_WAKE);
        MAP_HibernateTamperEnable();
    }

    /* Wait for the Hibernate entry flag to be set */
    while(!setHibEntry)
    {
    }

    /* Request for Hibernate Entry */
    MAP_HibernateRequest();

    while(1)
    {
    }
}
