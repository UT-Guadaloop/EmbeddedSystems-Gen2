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
 * MSP432E4 Example Project for accessing EEPROM
 *
 * Description: In the example EEPROM Word 0 to 31 is written once every second.
 * When the User Switch SW1 is pressed the EEPROM is erased and when User
 * Switch SW2 is pressed the EEPROM Word 0 to 31 are read and displayed.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
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

/* Utils Includes */
#include "uartstdio.h"

/* Define for count */
#define EEPROM_WORDLIMIT 32
#define SYSTICK_CNTMS    10

/* Custom define for EEPROM Operation State*/
#define EEPROM_READREQ   0x0
#define EEPROM_ERASEREQ  0x1

/**/
volatile uint8_t getNextEEPROMOperation = 0;
volatile uint8_t setTickCount = 0;

void SysTick_Handler(void)
{
    setTickCount++;
}

void GPIOJ_IRQHandler(void)
{
    uint32_t getSwitchState;

    /* Read the interrupt status register to find which switch button was
     * pressed and clear the interrupt status */
    getSwitchState = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    MAP_GPIOIntClear(GPIO_PORTJ_BASE, getSwitchState);

    /* If User Switch SW1 is pressed set the flag for erase request */
    if(getSwitchState & GPIO_PIN_0)
    {
        HWREGBITB(&getNextEEPROMOperation, EEPROM_ERASEREQ) = 1;
    }

    /* If User Switch SW2 is pressed set the flag for read request */
    if(getSwitchState & GPIO_PIN_1)
    {
        HWREGBITB(&getNextEEPROMOperation, EEPROM_READREQ) = 1;
    }

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
    uint32_t setCurrTime = 0;
    uint8_t  ii;
    uint32_t retInitStatus;
    uint32_t setDataforEEPROM[EEPROM_WORDLIMIT];
    uint32_t getDatafromEEPROM[EEPROM_WORDLIMIT];

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
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    GPIOJ->PUR = GPIO_PIN_0 | GPIO_PIN_1;

    /* Configure Interrupt Generation by Port Pin PJ0-PJ1 as falling edge */
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, (GPIO_PIN_0 | GPIO_PIN_1),
                       GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, (GPIO_INT_PIN_0 | GPIO_INT_PIN_1));
    MAP_IntEnable(INT_GPIOJ);

    /* Enable the EEPROM Module and Initialize the EEPROM Block */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)))
    {
    }

    retInitStatus = MAP_EEPROMInit();

    /* If EEPROM did not initialize then exit the program code */
    if(retInitStatus != EEPROM_INIT_OK)
    {
        UARTprintf("Error Initializing EEPROM\n");
        return 0;
    }

    UARTprintf("EEPROM Test Started\n");
    UARTprintf("Press USR_SW1 for Mass Erase \n");
    UARTprintf("Press USR_SW2 for Reading the current EEPROM Data\n");

    /* Enable the SysTick timer to generate an interrupt every 1/10 second */
    MAP_SysTickPeriodSet(systemClock/SYSTICK_CNTMS);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();

    while(1)
    {
        /* Wait for 1 second and clear the variable to 0*/
        while(setTickCount != SYSTICK_CNTMS)
        {
        }

        setTickCount = 0;
        setCurrTime++;

        /* Check if a read request is pending */
        if(HWREGBITB(&getNextEEPROMOperation, EEPROM_READREQ) == 1)
        {
            /* read the EEPROM to an array and print it to the UART */
            MAP_EEPROMRead(&getDatafromEEPROM[0], 0, EEPROM_WORDLIMIT*4);

            UARTprintf("Read EEPROM Data...\n");

            for(ii = 0; ii < EEPROM_WORDLIMIT; ii++)
            {
                UARTprintf("EEPROM Word %d = 0x%08x\n",ii ,getDatafromEEPROM[ii]);
            }

            /* Now clear the flag so that the loop is not re-entered */
            HWREGBITB(&getNextEEPROMOperation, EEPROM_READREQ) = 0;
        }
        else if(HWREGBITB(&getNextEEPROMOperation, EEPROM_ERASEREQ) == 1)
        {
            UARTprintf("Running EEPROM Erase and then Read Data...\n");

            /* Reset the Current Ticker Time */
            setCurrTime = 0;

            /* Erase the EEPROM */
            retInitStatus = MAP_EEPROMMassErase();

            if(retInitStatus != 0)
            {
                UARTprintf("ERROR Erasing EEPROM. EEPROM ERROR Code 0x%08x", retInitStatus);
                break;
            }

            /* read the EEPROM to an array and print it to the UART */
            MAP_EEPROMRead(&getDatafromEEPROM[0], 0, EEPROM_WORDLIMIT*4);

            for(ii = 0; ii < EEPROM_WORDLIMIT; ii++)
            {
                UARTprintf("EEPROM Word %d = 0x%08x\n",ii ,getDatafromEEPROM[ii]);
            }

            /* Now clear the flag so that the loop is not re-entered */
            HWREGBITB(&getNextEEPROMOperation, EEPROM_ERASEREQ) = 0;
        }
        else
        {
            /* The Current Ticker Time left shifted and OR-ed with the Word
             * count is the unique value written to each location */
            for(ii = 0; ii < EEPROM_WORDLIMIT; ii++)
            {
                setDataforEEPROM[ii] = (setCurrTime << 8 | ii);
            }

            /* Program EEPROM from Word 0 to 32 */
            MAP_EEPROMProgram(&setDataforEEPROM[0], 0, EEPROM_WORDLIMIT*4);
        }
    }
}
