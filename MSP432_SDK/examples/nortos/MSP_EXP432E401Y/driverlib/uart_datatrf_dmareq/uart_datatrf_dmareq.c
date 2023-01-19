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
 * MSP432E4 example code for UART data transfer with DMA.
 *
 * Description: The UART0 is configured to generate a DMA request when data is
 * received. The DMA controller then reads the DATA from the UART controller
 * and sends it back to the Console.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PA0|<-- U0RX
 *            |               PA1|--> U0TX
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

void UART0_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Read the interrupt status from the UART */
    getIntStatus = MAP_UARTIntStatus(UART0_BASE, true);

    /* If the UARTRX DMA Done interrupt is asserted then re-enable the DMA
     * control structure and clear the interrupt condition */
    if((getIntStatus & UART_INT_DMARX) == UART_INT_DMARX)
    {
        MAP_UARTIntClear(UART0_BASE, getIntStatus);

        MAP_uDMAChannelTransferSet(UDMA_CH8_UART0RX | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)&UART0->DR, (void *)&UART0->DR,
                                   100);
        MAP_uDMAChannelEnable(UDMA_CH8_UART0RX);
    }
}

int main(void)
{
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {

    }

    /* Configure the GPIO PA0-1 as U0RX-U0TX */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, (GPIO_PIN_0 | GPIO_PIN_1));

    /* Enable the clock to the UART-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)))
    {
    }

    /* Configure UART for 115200 bps, 8-N-1 format. Enable the DMA Request
     * generation and interrupt on DMARX Done. Also make sure that the FIFO is
     * disabled. */
    MAP_UARTConfigSetExpClk(UART0_BASE, systemClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE |
                             UART_CONFIG_STOP_ONE));
    MAP_UARTDMAEnable(UART0_BASE, UART_DMA_RX);
    MAP_UARTIntEnable(UART0_BASE, UART_INT_DMARX);
    MAP_UARTEnable(UART0_BASE);
    MAP_UARTFIFODisable(UART0_BASE);

    MAP_IntEnable(INT_UART0);

    /* Enable the DMA and Configure Channel for UART0 RX for Basic mode of
     * transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the UART0 RX DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH8_UART0RX);

    /* Put the attributes in a known state for the uDMA UART0 RX channel.
     * These should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH8_UART0RX,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the UART0 RX channel. The primary control structure is used for copying
     * the data from srcBuffer to UART0 Data register. The transfer data size
     * is 32 bits and the source & destination address are not incremented. */
    MAP_uDMAChannelControlSet(UDMA_CH8_UART0RX | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the UART0 RX primary control
     * structure. The mode is Ping-Pong mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH8_UART0RX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *)&UART0->DR, (void *)&UART0->DR,
                               100);

    /* The uDMA UART0 RX channel is primed to start a transfer. As soon as
     * the channel is enabled and the UART0 receives a byte of data it will
     * issue a DMA Request, and data transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH8_UART0RX);

    while(1)
    {
    }
}
