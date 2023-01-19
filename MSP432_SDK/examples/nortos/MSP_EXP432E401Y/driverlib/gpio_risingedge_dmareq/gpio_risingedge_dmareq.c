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
 * MSP432E4 Example Project to demonstrate GPIO rising edge DMA Request.
 *
 * Description: In this example the GPIO Port J Pin-0 is configured to generate
 * a DMA Request on rising edge detect. When the user releases the USR_SW1 the
 * GPIO for LED D2 is toggled by the DMA writing to the GPIO Port N Data
 * Register.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PN0|--> LED D2
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PJ0|<-- USR_SW1
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

static uint32_t srcBufferA = 0x1;
static uint32_t srcBufferB = 0x0;

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

void GPIOJ_IRQHandler(void)
{
    uint32_t getIntStatus;
    uint32_t getDMAStatus;

    /* Get the interrupt status from the GPIO and clear the status */
    getIntStatus = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);
    MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

    /* Read the primary and alternate control structures to find out which
     * of the structure has completed and generated the done interrupt. Then
     * re-initialize the appropriate structure */
    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH21_GPIOJ |
                                          UDMA_PRI_SELECT);

    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH21_GPIOJ | UDMA_PRI_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&srcBufferA, (void *)&GPION->DATA,
                                   sizeof(srcBufferA)/4);

    }

    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH21_GPIOJ |
                                          UDMA_ALT_SELECT);

    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH21_GPIOJ | UDMA_ALT_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&srcBufferB, (void *)&GPION->DATA,
                                   sizeof(srcBufferB)/4);

    }
}

int main(void)
{
    /* Configure the system clock for 120 MHz */
    MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                            120000000);

    /* Enable the clock to the GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN0 as output */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0 as input with internal pull up enabled.
     * Configure the PJ0 for a rising edge DMA Request detection */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR |= GPIO_PIN_0;
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_RISING_EDGE);
    MAP_GPIODMATriggerEnable(GPIO_PORTJ_BASE, GPIO_PIN_0);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_DMA);

    /* Enable the DMA and Configure Channel for GPIO Port J for Ping Pong mode
     * of transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the GPIO Port J DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH21_GPIOJ);

    /* Put the attributes in a known state for the uDMA GPIO Port J channel.
     * These should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH21_GPIOJ,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the GPIO Port J channel. The primary control structure is used for
     * copying the data from srcBufferA to GPIO Port N Data register. The
     * transfer data size is 32 bits and the source & destination address are
     * not incremented. */
    MAP_uDMAChannelControlSet(UDMA_CH21_GPIOJ | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the GPIO Port J primary control
     * structure. The mode is Ping-Pong mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH21_GPIOJ | UDMA_PRI_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)&srcBufferA, (void *)&GPION->DATA,
                               sizeof(srcBufferA)/4);

    /* Configure the control parameters for the alternate control structure for
     * the GPIO Port J channel. The primary control structure is used for
     * copying the data from srcBufferB to GPIO Port N Data register. The
     * transfer data size is 32 bits and the source & destination address are
     * not incremented. */
    MAP_uDMAChannelControlSet(UDMA_CH21_GPIOJ | UDMA_ALT_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the GPIO Port J primary control
     * structure. The mode is Ping-Pong mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH21_GPIOJ | UDMA_ALT_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)&srcBufferB, (void *)&GPION->DATA,
                               sizeof(srcBufferB)/4);

    /* The uDMA GPIO Port J channel is primed to start a transfer. As soon as
     * the channel is enabled and the GPIO Port J will issue a DMA Request,
     * when the GPIO edge is detected and data transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH21_GPIOJ);

    /* Enable GPIO Port J Interrupt */
    MAP_IntEnable(INT_GPIOJ);

    while(1)
    {
        
    }
}
