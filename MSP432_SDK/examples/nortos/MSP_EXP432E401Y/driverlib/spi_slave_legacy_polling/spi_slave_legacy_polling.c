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
 * MSP432E4 Example Project for SSI-Slave in Legacy Mode with pollimg.
 *
 * Description: This application example configures the SSI module for slave
 * mode operation in Motorola Frame Format-0. The use of the example requires
 * another MSP-EXP432E401Y board to be running the spi_master_legacy_polling
 * application. The master board sends a data to the slave and the slave
 * inverts the bits. The master board reads the data from the slave and
 * compares the read data from the slave with the inverted master data.
 *
 *                MSP432E401Y                      MSP432E401Y
 *             ------------------               ------------------
 *         /|\|      SLAVE       |             |      MASTER      |
 *          | |                  |             |                  |
 *          --|RST            PA2|<--SSI0CLK<--|PA2               |
 *            |               PA3|<--SSI0FSS<--|PA3               |
 *            |               PA5|<--SSI0 TX<--|PA4               |
 *            |               PA4|-->SSI0 RX-->|PA5               |
 *            |                  |             |                  |
 *            |                  |             |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_SSI_DATA   4
#define HEAD_DATA_PATTERN 0x55

int main(void)
{
    uint8_t ii;
    uint8_t  sendDummyData = 0x00;
    uint32_t getData[NUM_OF_SSI_DATA]   = {HEAD_DATA_PATTERN, 0x00, 0x00, 0x00};
    uint32_t getResponseData;
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable clocks to GPIO Port A and configure pins as SSI */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    MAP_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    MAP_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    MAP_GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);
    MAP_GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);
    MAP_GPIOPinTypeSSI(GPIO_PORTA_BASE, (GPIO_PIN_2 | GPIO_PIN_3 |
                                         GPIO_PIN_4 | GPIO_PIN_5));

    /* Enable the clock to SSI-0 module and configure the SSI Master */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)))
    {
    }

    MAP_SSIConfigSetExpClk(SSI0_BASE, systemClock, SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_SLAVE, (systemClock/24), 8);
    MAP_SSIEnable(SSI0_BASE);

    /* Flush the Receive FIFO */
    while(MAP_SSIDataGetNonBlocking(SSI0_BASE, &getResponseData));

    while(1)
    {
        /* Wait for the header data packet to come */
        do
        {
            MAP_SSIDataGet(SSI0_BASE, &getResponseData);
        } while(getResponseData != HEAD_DATA_PATTERN);

        /* Read the rest of the packet and invert the data */
        for(ii = 1; ii < NUM_OF_SSI_DATA; ii++)
        {
            MAP_SSIDataGet(SSI0_BASE, &getData[ii]);
            getData[ii] ^= 0xFF;
        }

        /* Now put the data into the FIFO for transmission to the master */
        for(ii = 0; ii < NUM_OF_SSI_DATA; ii++)
        {
            MAP_SSIDataPut(SSI0_BASE, getData[ii]);
        }

        /* Now put the dummy packet */
        MAP_SSIDataPut(SSI0_BASE, sendDummyData);

        /* Clear the payload */
        for(ii = 1; ii < NUM_OF_SSI_DATA; ii++)
        {
            getData[ii] = 0x00;
        }

    }
}
