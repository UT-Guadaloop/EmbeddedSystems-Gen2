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
 * MSP432E4 Example project for Configuring a timer in 32-bit periodic mode
 *
 * Description: In this example, the timer is configured to generate an
 * interrupt every 1 second in 32-bit periodic mode. On the interrupt the state
 * of the LED is toggled.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |               PN0|-->LED
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

void TIMER0A_IRQHandler(void)
{
    uint32_t getTimerInterrupt;
    uint32_t getDMAStatus;

    /* Get timer interrupt status  and clear the same */
    getTimerInterrupt = MAP_TimerIntStatus(TIMER0_BASE, true);
    MAP_TimerIntClear(TIMER0_BASE, getTimerInterrupt);

    /* Read the primary and alternate control structures to find out which
     * of the structure has completed and generated the done interrupt. Then
     * re-initialize the appropriate structure */
    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH18_TIMER0A |
                                          UDMA_PRI_SELECT);

    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH18_TIMER0A | UDMA_PRI_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&srcBufferA, (void *)&GPION->DATA,
                                   sizeof(srcBufferA)/4);

    }

    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH18_TIMER0A |
                                          UDMA_ALT_SELECT);

    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH18_TIMER0A | UDMA_ALT_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&srcBufferB, (void *)&GPION->DATA,
                                   sizeof(srcBufferB)/4);

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
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }

    /* Configure the GPIO PN0 as output and put in low state */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));

    /* Enable the Timer-0 in 32-bit periodic mode with DMA Request generated
     * every 1 second */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, systemClock);

    /* Enable the DMA Done Interrupt bit from the Timer */
    MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_DMA);

    MAP_TimerDMAEventSet(TIMER0_BASE, TIMER_DMA_TIMEOUT_A);

    /* Enable the DMA and Configure Channel for TIMER0A for Ping Pong mode of
     * transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the Timer 0A DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH18_TIMER0A);

    /* Put the attributes in a known state for the uDMA TIMER0A channel. These
     * should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH18_TIMER0A,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the TIMER0A channel. The primary control structure is used for copying
     * the data from srcBufferA to GPIO Port N Data register. The transfer
     * data size is 32 bits and the source & destination address are not
     * incremented.
     */
    MAP_uDMAChannelControlSet(UDMA_CH18_TIMER0A | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the TIMER alternate control structure.
     * The mode is Ping-Pong mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH18_TIMER0A | UDMA_PRI_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)&srcBufferA, (void *)&GPION->DATA,
                               sizeof(srcBufferA)/4);

    /* Configure the control parameters for the alternate control structure for
     * the TIMER0A channel. The primary control structure is used for copying
     * the data from srcBufferB to GPIO Port N Data register. The transfer
     * data size is 32 bits and the source & destination address are not
     * incremented.
     */
    MAP_uDMAChannelControlSet(UDMA_CH18_TIMER0A | UDMA_ALT_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the TIMER primary control structure.
     * The mode is Ping-Pong mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH18_TIMER0A | UDMA_ALT_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)&srcBufferB, (void *)&GPION->DATA,
                               sizeof(srcBufferB)/4);

    /* The uDMA TIMER channel is primed to start a transfer. As soon as the
     * channel is enabled and the Timer will issue a DMA Request, the data
     * transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH18_TIMER0A);

    /* Enable Timer Interrupt */
    MAP_IntEnable(INT_TIMER0A);

    /* Start the timer */
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

    while(1)
    {
    }
}
