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
 * MSP432E4 example code for EPI to access SDRAM memory using DMA.
 *
 * Description: The EPI is configured to access an SDRAM memory at 60MHz. The
 * example programs the GPIOs for EPI and configures the EPI. After the
 * initialization is complete, the EPI is configured to generate a DMA TX
 * request to write an internal buffer to the SDRAM memory. On completion the
 * non-blocking read FIFO is configured to read the data from the SDRAM to
 * another internal buffer. The resulting data check is printed on the console
 * along with the throughput.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            EPI|-->SDRAM
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

/* Defines for the scope of the example */
#define SDRAM_START_ADDRESS   0x00000000
#define SDRAM_END_ADDRESS     0x01FFFFFF
#define NO_OF_RAM_LOC         512
#define SDRAM_MAPPING_ADDRESS 0x60000000
#define SYSTICK_MAX_COUNT     16777216

/* The control table used by the uDMA controller.  This table must be aligned
 * to a 1024 byte boundary. */
#if defined(__ICCARM__)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

/* Global Variables */
uint32_t internalWriteBuf[NO_OF_RAM_LOC];
uint32_t internalReadBuf[NO_OF_RAM_LOC];
bool     setTransmitDone = false;
bool     setReceiveDone = false;

void EPI0_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status */
    getIntStatus = MAP_EPIIntStatus(EPI0_BASE, true);

    if((getIntStatus & EPI_INT_DMA_TX_DONE) == EPI_INT_DMA_TX_DONE)
    {
        /* Clear the interrupt status and set the flag for the main code to
         * proceed */
        MAP_EPIIntErrorClear(EPI0_BASE, EPI_INT_ERR_DMAWRIC);
        setTransmitDone = true;
    }

    if((getIntStatus & EPI_INT_DMA_RX_DONE) == EPI_INT_DMA_RX_DONE)
    {
        /* Clear the interrupt status and set the flag for the main code to
         * proceed */
        MAP_EPIIntErrorClear(EPI0_BASE, EPI_INT_ERR_DMARDIC);
        setReceiveDone = true;
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
    uint32_t ii;
    uint32_t getStartTime, getEndTime;
    uint32_t systemClockinMHz;
    uint32_t *sdram32bitAddrPointer;
    bool     setDataCheck = false;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Enable the clock to the GPIO Ports that are required for EPI */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    MAP_GPIOPinConfigure(GPIO_PH0_EPI0S0);
    MAP_GPIOPinConfigure(GPIO_PH1_EPI0S1);
    MAP_GPIOPinConfigure(GPIO_PH2_EPI0S2);
    MAP_GPIOPinConfigure(GPIO_PH3_EPI0S3);
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
    MAP_GPIOPinConfigure(GPIO_PP3_EPI0S30);
    MAP_GPIOPinConfigure(GPIO_PK5_EPI0S31);

    MAP_GPIOPinTypeEPI(GPIO_PORTA_BASE, (GPIO_PIN_7 | GPIO_PIN_6));
    MAP_GPIOPinTypeEPI(GPIO_PORTB_BASE, (GPIO_PIN_3));
    MAP_GPIOPinTypeEPI(GPIO_PORTC_BASE, (GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 |
                                         GPIO_PIN_4));
    MAP_GPIOPinTypeEPI(GPIO_PORTG_BASE, (GPIO_PIN_1 | GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTH_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTK_BASE, (GPIO_PIN_5));
    MAP_GPIOPinTypeEPI(GPIO_PORTL_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTM_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTP_BASE, (GPIO_PIN_3 | GPIO_PIN_2));

    /* Enable the clock to the EPI and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPI0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EPI0)))
    {
    }

    /* Configure the EPI to access the SDRAM memory at 60 MHz Set the EPI
     * clock to half the system clock. */
    MAP_EPIDividerSet(EPI0_BASE, 1);

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

    /* Set the EPI memory pointer to the base of EPI memory space.  Note that
     * sdram32bitAddrPointer is declared as volatile so the compiler should not
     * optimize reads out of the memory.  With this pointer, the memory space
     * is accessed like a simple array. */
    sdram32bitAddrPointer = (uint32_t *)SDRAM_MAPPING_ADDRESS;

    /* Read the initial data in SDRAM, and display it on the console. */
    UARTprintf("  SDRAM Initial Data:\n");
    UARTprintf("     Mem[0x6000.0000] = 0x%8x\n",
               sdram32bitAddrPointer[SDRAM_START_ADDRESS]);
    UARTprintf("     Mem[0x603F.FFFF] = 0x%8x\n\n",
               sdram32bitAddrPointer[SDRAM_END_ADDRESS]);

    /* Display what writes we are doing on the console. */
    UARTprintf("  SDRAM Write:\n");
    UARTprintf("     Mem[0x6000.0000] <- 0xabcd1234\n");
    UARTprintf("     Mem[0x603F.FFFF] <- 0x4321dcba\n\n");

    /* Write to the first 2 and last 2 address of the SDRAM card.  Since the
     * SDRAM card is word addressable, we will write words. */
    sdram32bitAddrPointer[SDRAM_START_ADDRESS] = 0xabcd1234;
    sdram32bitAddrPointer[SDRAM_END_ADDRESS] = 0x4321dcba;

    /* Read back the data you wrote, and display it on the console. */
    UARTprintf("  SDRAM Read:\n");
    UARTprintf("     Mem[0x6000.0000] = 0x%8x\n",
               sdram32bitAddrPointer[SDRAM_START_ADDRESS]);
    UARTprintf("     Mem[0x603F.FFFF] = 0x%8x\n\n",
               sdram32bitAddrPointer[SDRAM_END_ADDRESS]);

    /* Check the integrity of the data. */
    if((sdram32bitAddrPointer[SDRAM_START_ADDRESS] == 0xabcd1234) &&
       (sdram32bitAddrPointer[SDRAM_END_ADDRESS] == 0x4321dcba))
    {
        /* Read and write operations were successful.  Return with no errors.
         *  */
        UARTprintf("Read and write to external SDRAM was successful!\n");
        UARTprintf("Begin Performance Test!\n");
    }
    else
    {
        /* Display on the console that there was an error. */
        UARTprintf("Read and/or write failure!");
        UARTprintf(" Check if your SDRAM card is plugged in.");
        return(0);
    }

    /* Update the internal write buffer with random data and clear the internal
     * read buffer */
    for(ii = 0; ii < NO_OF_RAM_LOC; ii++)
    {
        internalWriteBuf[ii] = rand() | (rand() << 16);
        internalReadBuf[ii]  = 0x0;
    }

    /* Configure the Write and Read Path DMA Request for EPI. The TX FIFO is
     * configured to generate a request when there are 2 locations available.
     * The RX FIFO is configured when the NBR FIFO is Half Full with 4 words.*/
    MAP_EPIFIFOConfig(EPI0_BASE, EPI_FIFO_CONFIG_TX_1_4 |
                                 EPI_FIFO_CONFIG_RX_1_2);

    MAP_EPINonBlockingReadConfigure(EPI0_BASE, 0, EPI_NBCONFIG_SIZE_32,
                                    0x60000000);

    MAP_EPIIntEnable(EPI0_BASE, (EPI_INT_DMA_TX_DONE | EPI_INT_DMA_RX_DONE));

    MAP_IntEnable(INT_EPI0);

    /* Enable the DMA and Configure Channel for EPI TX and RX in BASIC mode of
     * transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the EPI RX and TX DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH20_EPI0RX);
    MAP_uDMAChannelAssign(UDMA_CH21_EPI0TX);

    /* Put the attributes in a known state for the uDMA EPI TX channel. These
     * should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH21_EPI0TX,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the EPI TX channel. The primary control structure is used for copying
     * the data from Internal buffer to SDRAM memory. The transfer data size
     * is 32 bits and the source & destination address are incremented. */
    MAP_uDMAChannelControlSet(UDMA_CH21_EPI0TX | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_32 | UDMA_DST_INC_32 |
                              UDMA_ARB_2);

    /* Set up the transfer parameters for the EPI TX alternate control
     * structure. The mode is Basic mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH21_EPI0TX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *)&internalWriteBuf, (void *)sdram32bitAddrPointer,
                               sizeof(internalWriteBuf)/4);

    /* Put the attributes in a known state for the uDMA EPI RX channel. These
     * should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH20_EPI0RX,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the alternate control structure for
     * the EPI RX channel. The primary control structure is used for copying
     * the data from EPIRADDR0 to internal buffer. The transfer data size is 32
     * bits and the source address is not incremented & destination address is
     * incremented. */
    MAP_uDMAChannelControlSet(UDMA_CH20_EPI0RX | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_32 |
                              UDMA_ARB_4);

    /* Set up the transfer parameters for the EPI RX primary control structure.
     * The mode is Basic mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH20_EPI0RX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *)&EPI0->READFIFO0, (void *)&internalReadBuf,
                               sizeof(internalReadBuf)/4);

    /* The uDMA EPI RX and TX channel is primed to start a transfer. As soon
     * as the channel is enabled and the EPI TX or RX count is programmed, EPI
     * will issue a DMA Request and the data transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH20_EPI0RX);
    MAP_uDMAChannelEnable(UDMA_CH21_EPI0TX);

    /* Compute the System Frequency with the MHz field adjusted for throughput
     *  calculation */
    systemClockinMHz = systemClock/1000000;

    /* Initialize and Enable SysTick Timer */
    MAP_SysTickPeriodSet(SYSTICK_MAX_COUNT);
    MAP_SysTickEnable();

    /* Get the Start Count */
    getStartTime = MAP_SysTickValueGet();

    /* Start the write from internal buffer to external SDRAM */
    MAP_EPIDMATxCount(EPI0_BASE, NO_OF_RAM_LOC);

    /* Wait for the TX DONE interrupt from the DMA to Get the End Count */
    while(!(setTransmitDone))
    {
    }
    getEndTime = MAP_SysTickValueGet();

    UARTprintf("32-bit SDRAM-DMA Write : %03d.%03d MBps\n",
               ((NO_OF_RAM_LOC*systemClockinMHz*4)/(getStartTime-getEndTime)),
               ((NO_OF_RAM_LOC*systemClockinMHz*4)%(getStartTime-getEndTime)));

    /* Get the Start Count */
    getStartTime = MAP_SysTickValueGet();

    /* Start the write from internal buffer to external SDRAM */
    MAP_EPINonBlockingReadStart(EPI0_BASE, 0, NO_OF_RAM_LOC);

    /* Wait for the TX DONE interrupt from the DMA to Get the End Count */
    while(!(setReceiveDone))
    {
    }
    getEndTime = MAP_SysTickValueGet();

    UARTprintf("32-bit SDRAM-DMA Read : %03d.%03d MBps\n",
               ((NO_OF_RAM_LOC*systemClockinMHz*4)/(getStartTime-getEndTime)),
               ((NO_OF_RAM_LOC*systemClockinMHz*4)%(getStartTime-getEndTime)));

    /* Perform Data Integrity Check */
    for(ii = 0; ii < NO_OF_RAM_LOC; ii++)
    {
        if(internalWriteBuf[ii] != internalReadBuf[ii])
        {
            UARTprintf("Data Integrity Check Failed !!!!\n");
            setDataCheck = true;
            break;
        }
    }

    if(!setDataCheck)
    {
        UARTprintf("Data Integrity Check Passed\n");
    }

    while(1)
    {
    }
}
