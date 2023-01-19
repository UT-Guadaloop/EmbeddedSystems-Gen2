/* --COPYRIGHT--,BSD
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 * MSP432E4 out of box example for EPI to access SDRAM memory
 *
 * Description: The EPI is configured to access an SDRAM memory at 60MHz. The
 * example programs the GPIOs for EPI and configures the EPI. After the
 * initialization is complete, basic SDRAM Writes and Reads are performed using
 * 16-bit, 32-bit and 64-bit. SysTick Timer is used to measure the throughput,
 * which is displayed on the UART Console using the settings:
 *      Baud-rate:  115200
 *      Data bits:       8
 *      Stop bits:       1
 *      Parity:       None
 *      Flow Control: None
 *
 * The green LED on the Ethernet Jack (PN0) is used as a "blinking LED" to
 * indicate that the application is running.
 * The orange LED (PN1) is used as a "status LED" that turns on when the test
 * is completed successfully.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            EPI|-->SDRAM
 *            |                  |
 *            |               PN0|--Green LED
 *            |               PN1|--Yellow LED
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

/* Defines for the scope of the example */
#define SDRAM_START_ADDRESS   0x00000000
#define SDRAM_END_ADDRESS     0x01FFFFFF
#define NO_OF_RAM_LOC         4096
#define SDRAM_MAPPING_ADDRESS 0x60000000

/* Define for setting up the system clock. */
#define SYSTICKHZ               10

/* Global Variables */
static volatile uint16_t *sdram16bitAddrPointer;
static volatile uint32_t *sdram32bitAddrPointer;
static volatile uint64_t *sdram64bitAddrPointer;
volatile uint16_t internalRamArray16bit[NO_OF_RAM_LOC/2];
volatile uint32_t internalRamArray32bit[NO_OF_RAM_LOC/4];
volatile uint64_t internalRamArray64bit[NO_OF_RAM_LOC/8];

/* The interrupt handler for the SysTick interrupt. */
void SysTick_Handler(void)
{
    /* Toggle LED. */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,
                     ((GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0)) ^
                      GPIO_PIN_0));
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
    uint32_t ii;
    uint32_t getStartTime, getEndTime;
    uint32_t systemClockinMHz;
    bool testStatus;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Print Banner */
    UARTprintf("\033[2J\033[H");
    UARTprintf("\n\nWelcome to the MSP432E411Y-BGAEVM's,"
               "\nOut of box Demo.\r\n\n");

    /* Configure SysTick for a periodic interrupt. This interrupt is used to
     * toggle the "blinking" LED. */
    MAP_SysTickPeriodSet(systemClock / SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    /* Configure Port N0 as an output and initialize it to "ON". This LED is
     * used as the "blinking" LED and toggled in the SysTick interrupt. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);

    /* Configure Port N1 as an output. This LED is used as the "status" LED and
     * turned on if the test passes. */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    /* Enable the clock to the GPIO Ports that are required for EPI */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    MAP_GPIOPinConfigure(GPIO_PH0_EPI0S0);
    MAP_GPIOPinConfigure(GPIO_PH1_EPI0S1);
    MAP_GPIOPinConfigure(GPIO_PH2_EPI0S2);
    MAP_GPIOPinConfigure(GPIO_PK3_EPI0S3);
    MAP_GPIOPinConfigure(GPIO_PC7_EPI0S4);
    MAP_GPIOPinConfigure(GPIO_PC6_EPI0S5);
    MAP_GPIOPinConfigure(GPIO_PC5_EPI0S6);
    MAP_GPIOPinConfigure(GPIO_PC4_EPI0S7);
    MAP_GPIOPinConfigure(GPIO_PA6_EPI0S8);
    MAP_GPIOPinConfigure(GPIO_PA7_EPI0S9);
    MAP_GPIOPinConfigure(GPIO_PG1_EPI0S10);
    MAP_GPIOPinConfigure(GPIO_PG0_EPI0S11);
    MAP_GPIOPinConfigure(GPIO_PM3_EPI0S12);
    MAP_GPIOPinConfigure(GPIO_PM2_EPI0S13);
    MAP_GPIOPinConfigure(GPIO_PM1_EPI0S14);
    MAP_GPIOPinConfigure(GPIO_PM0_EPI0S15);
    MAP_GPIOPinConfigure(GPIO_PL0_EPI0S16);
    MAP_GPIOPinConfigure(GPIO_PL1_EPI0S17);
    MAP_GPIOPinConfigure(GPIO_PL2_EPI0S18);
    MAP_GPIOPinConfigure(GPIO_PL3_EPI0S19);
    MAP_GPIOPinConfigure(GPIO_PB3_EPI0S28);
    MAP_GPIOPinConfigure(GPIO_PP2_EPI0S29);
    MAP_GPIOPinConfigure(GPIO_PN3_EPI0S30);
    MAP_GPIOPinConfigure(GPIO_PK5_EPI0S31);

    MAP_GPIOPinTypeEPI(GPIO_PORTA_BASE, (GPIO_PIN_7 | GPIO_PIN_6));
    MAP_GPIOPinTypeEPI(GPIO_PORTB_BASE, (GPIO_PIN_3));
    MAP_GPIOPinTypeEPI(GPIO_PORTC_BASE, (GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 |
                                         GPIO_PIN_4));
    MAP_GPIOPinTypeEPI(GPIO_PORTG_BASE, (GPIO_PIN_1 | GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTH_BASE, (GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTK_BASE, (GPIO_PIN_3 | GPIO_PIN_5));
    MAP_GPIOPinTypeEPI(GPIO_PORTL_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTM_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTP_BASE, (GPIO_PIN_2));
    MAP_GPIOPinTypeEPI(GPIO_PORTN_BASE, (GPIO_PIN_3));

    /* Enable the clock to the EPI and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPI0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EPI0)))
    {
    }

    /* Configure the EPI to access the SDRAM memory at 60 MHz Set the EPI
     * clock to half the system clock. */
    MAP_EPIDividerSet(EPI0_BASE, 2);

    /* Sets the usage mode of the EPI module.  For this example we will use
     * the SDRAM mode to talk to the external 64MB SDRAM daughter card. */
    MAP_EPIModeSet(EPI0_BASE, EPI_MODE_SDRAM);

    /* Configure the SDRAM mode.  We configure the SDRAM according to our core
     * clock frequency.  We will use the normal (or full power) operating
     * state which means we will not use the low power self-refresh state.
     * Set the SDRAM size to 64MB with a refresh interval of 468 clock ticks*/
    MAP_EPIConfigSDRAMSet(EPI0_BASE, (EPI_SDRAM_CORE_FREQ_50_100 |
                                      EPI_SDRAM_FULL_POWER |
                                      EPI_SDRAM_SIZE_512MBIT), 468);

    /* Set the address map.  The EPI0 is mapped from 0x60000000 to 0x01FFFFFF.
     * For this example, we will start from a base address of 0x60000000 with
     * a size of 256MB.  Although our SDRAM is only 64MB, there is no 64MB
     * aperture option so we pick the next larger size. */
    MAP_EPIAddressMapSet(EPI0_BASE, EPI_ADDR_RAM_SIZE_256MB | EPI_ADDR_RAM_BASE_6);

    /* Wait for the SDRAM wake-up to complete by polling the SDRAM
     * initialization sequence bit.  This bit is true when the SDRAM interface
     * is going through the initialization and false when the SDRAM interface
     * it is not in a wake-up period. */
    while(EPI0->STAT & EPI_STAT_INITSEQ)
    {
    }

    /* Initialize the testStatus variable. Assume that the test has failed. */
    testStatus = 0;

    /* Set the EPI memory pointer to the base of EPI memory space.  Note that
     * sdram16bitAddrPointer is declared as volatile so the compiler should not
     * optimize reads out of the memory.  With this pointer, the memory space
     * is accessed like a simple array. */
    sdram16bitAddrPointer = (uint16_t *)0x60000000;
    sdram32bitAddrPointer = (uint32_t *)0x60000000;
    sdram64bitAddrPointer = (uint64_t *)0x60000000;

    /* Read the initial data in SDRAM, and display it on the console. */
    UARTprintf("\033[2CSDRAM Initial Data:\n");
    UARTprintf("\033[5CMem[0x6000.0000] = 0x%4x\n",
               sdram16bitAddrPointer[SDRAM_START_ADDRESS]);
    UARTprintf("\033[5CMem[0x6000.0001] = 0x%4x\n",
               sdram16bitAddrPointer[SDRAM_START_ADDRESS + 1]);
    UARTprintf("\033[5CMem[0x603F.FFFE] = 0x%4x\n",
               sdram16bitAddrPointer[SDRAM_END_ADDRESS - 1]);
    UARTprintf("\033[5CMem[0x603F.FFFF] = 0x%4x\n\n",
               sdram16bitAddrPointer[SDRAM_END_ADDRESS]);

    /* Display what writes we are doing on the console. */
    UARTprintf("\033[2CSDRAM Write:\n");
    UARTprintf("\033[5CMem[0x6000.0000] <- 0xabcd\n");
    UARTprintf("\033[5CMem[0x6000.0001] <- 0x1234\n");
    UARTprintf("\033[5CMem[0x603F.FFFE] <- 0xdcba\n");
    UARTprintf("\033[5CMem[0x603F.FFFF] <- 0x4321\n\n");

    /* Write to the first 2 and last 2 address of the SDRAM card.  Since the
     * SDRAM card is word addressable, we will write words. */
    sdram16bitAddrPointer[SDRAM_START_ADDRESS] = 0xabcd;
    sdram16bitAddrPointer[SDRAM_START_ADDRESS + 1] = 0x1234;
    sdram16bitAddrPointer[SDRAM_END_ADDRESS - 1] = 0xdcba;
    sdram16bitAddrPointer[SDRAM_END_ADDRESS] = 0x4321;

    /* Read back the data you wrote, and display it on the console. */
    UARTprintf("\033[2CSDRAM Read:\n");
    UARTprintf("\033[5CMem[0x6000.0000] = 0x%4x\n",
               sdram16bitAddrPointer[SDRAM_START_ADDRESS]);
    UARTprintf("\033[5CMem[0x6000.0001] = 0x%4x\n",
               sdram16bitAddrPointer[SDRAM_START_ADDRESS + 1]);
    UARTprintf("\033[5CMem[0x603F.FFFE] = 0x%4x\n",
               sdram16bitAddrPointer[SDRAM_END_ADDRESS - 1]);
    UARTprintf("\033[5CMem[0x603F.FFFF] = 0x%4x\n\n",
               sdram16bitAddrPointer[SDRAM_END_ADDRESS]);

    /* Check the integrity of the data. */
    if((sdram16bitAddrPointer[SDRAM_START_ADDRESS] == 0xabcd) &&
       (sdram16bitAddrPointer[SDRAM_START_ADDRESS + 1] == 0x1234) &&
       (sdram16bitAddrPointer[SDRAM_END_ADDRESS - 1] == 0xdcba) &&
       (sdram16bitAddrPointer[SDRAM_END_ADDRESS] == 0x4321))
    {
        /* Read and write operations were successful.  Return with no errors.
         *  */
        UARTprintf("Read and write to external SDRAM was successful!\n\n");
        UARTprintf("\033[2CBegin Performance Test\n");
        testStatus = 1;
    }
    else
    {
        /* Display on the console that there was an error. */
        UARTprintf("Read and/or write failure!\n"
                   " Check the connection to the SDRAM (U2).");
        testStatus = 0;
    }

    /* If the read and write to SDRAM was successful then run the performance
     * test*/
    if(testStatus != 0)
    {
        /* Compute the System Frequency with the MHz field adjusted for throughput
         * calculation */
        systemClockinMHz = systemClock / 1000000;

        /* Get the Start Count */
        getStartTime = MAP_SysTickValueGet();

        /* Begin Writing a fixed pattern to external SDRAM */
        for(ii = 0; ii < (NO_OF_RAM_LOC / 2); ii++)
        {
            sdram16bitAddrPointer[SDRAM_START_ADDRESS + ii] = 0xabcd1234 + ii;
        }

        /* Get the End Count */
        getEndTime = MAP_SysTickValueGet();

        UARTprintf("\033[5C16-bit SDRAM Write : %02d.%03d MBps\n",
                   ((NO_OF_RAM_LOC * systemClockinMHz) / (getStartTime - getEndTime)),
                   ((NO_OF_RAM_LOC * systemClockinMHz) % (getStartTime - getEndTime)));

        /* Get the Start Count */
        getStartTime = MAP_SysTickValueGet();

        /* Begin Reading the fixed pattern from external SDRAM */
        for(ii = 0; ii < (NO_OF_RAM_LOC / 2); ii++)
        {
            internalRamArray16bit[ii] = sdram16bitAddrPointer[SDRAM_START_ADDRESS + ii];
        }

        /* Get the End Count */
        getEndTime = MAP_SysTickValueGet();

        UARTprintf("\033[5C16-bit SDRAM Read  : %02d.%03d MBps\n",
                   ((NO_OF_RAM_LOC * systemClockinMHz) / (getStartTime - getEndTime)),
                   ((NO_OF_RAM_LOC * systemClockinMHz) % (getStartTime - getEndTime)));

        /* Get the Start Count */
        getStartTime = MAP_SysTickValueGet();

        /* Begin Writing a fixed pattern to external SDRAM */
        for(ii = 0; ii < (NO_OF_RAM_LOC / 4); ii++)
        {
            sdram32bitAddrPointer[SDRAM_START_ADDRESS+ii] = 0xabcd1234+ii;
        }

        /* Get the End Count */
        getEndTime = MAP_SysTickValueGet();

        UARTprintf("\033[5C32-bit SDRAM Write : %02d.%03d MBps\n",
                   ((NO_OF_RAM_LOC * systemClockinMHz) / (getStartTime - getEndTime)),
                   ((NO_OF_RAM_LOC * systemClockinMHz) % (getStartTime - getEndTime)));

        /* Get the Start Count */
        getStartTime = MAP_SysTickValueGet();

        /* Begin Reading the fixed pattern from external SDRAM */
        for(ii = 0; ii < (NO_OF_RAM_LOC / 4); ii++)
        {
            internalRamArray32bit[ii] = sdram32bitAddrPointer[SDRAM_START_ADDRESS+ii];
        }

        /* Get the End Count */
        getEndTime = MAP_SysTickValueGet();

        UARTprintf("\033[5C32-bit SDRAM Read  : %02d.%03d MBps\n",
                   ((NO_OF_RAM_LOC * systemClockinMHz) / (getStartTime - getEndTime)),
                   ((NO_OF_RAM_LOC * systemClockinMHz) % (getStartTime - getEndTime)));

        /* Get the Start Count */
        getStartTime = MAP_SysTickValueGet();

        /* Begin Writing a fixed pattern to external SDRAM */
        for(ii = 0; ii < (NO_OF_RAM_LOC / 8); ii++)
        {
            sdram64bitAddrPointer[SDRAM_START_ADDRESS+ii] = 0xabcd1234+ii;
        }

        /* Get the End Count */
        getEndTime = MAP_SysTickValueGet();

        UARTprintf("\033[5C64-bit SDRAM Write : %02d.%03d MBps\n",
                   ((NO_OF_RAM_LOC * systemClockinMHz) / (getStartTime - getEndTime)),
                   ((NO_OF_RAM_LOC * systemClockinMHz) % (getStartTime - getEndTime)));

        /* Get the Start Count */
        getStartTime = MAP_SysTickValueGet();

        /* Begin Reading the fixed pattern from external SDRAM */
        for(ii = 0; ii < (NO_OF_RAM_LOC / 8); ii++)
        {
            internalRamArray64bit[ii] = sdram64bitAddrPointer[SDRAM_START_ADDRESS+ii];
        }

        /* Get the End Count */
        getEndTime = MAP_SysTickValueGet();

        UARTprintf("\033[5C64-bit SDRAM Read  : %02d.%03d MBps\n",
                   ((NO_OF_RAM_LOC * systemClockinMHz) / (getStartTime - getEndTime)),
                   ((NO_OF_RAM_LOC * systemClockinMHz) % (getStartTime - getEndTime)));

        /* Let the user know that the Test has completed successfully. */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
    }

    while(1)
    {
    }
}
