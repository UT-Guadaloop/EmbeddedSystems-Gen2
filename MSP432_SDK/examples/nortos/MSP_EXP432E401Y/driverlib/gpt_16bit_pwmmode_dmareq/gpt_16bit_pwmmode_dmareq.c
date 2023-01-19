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
 * MSP432E4 Example project for Configuring a timer in 16-bit PWM mode with DMA
 * Request
 *
 * Description: In this example, the timer is configured to generate a PWM
 * output with a frequency of 2 KHz and 66% duty cycle. The DMA Request is
 * generated on every rising edge of the PWM signal. The DMA request is used
 * to copy data from one SRAM buffer to another in AUTO Mode.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |               PA7|-->PWM
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

#define MEM_BUF_SIZE     16

static uint32_t srcBuffer[MEM_BUF_SIZE] = {0x11111111, 0x22222222, 0x33333333, 0x44444444,
                                           0x55555555, 0x66666666, 0x77777777, 0x88888888,
                                           0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC,
                                           0xDDDDDDDD, 0xEEEEEEEE, 0xFFFFFFFF, 0xCA5ECA5E};
static uint32_t dstBuffer[MEM_BUF_SIZE];

volatile bool bSetEventFlag;

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

void TIMER3B_IRQHandler(void)
{
    uint32_t getTimerIntStatus;

    /* Get the timer interrupt status and clear the same */
    getTimerIntStatus = MAP_TimerIntStatus(TIMER3_BASE, true);

    MAP_TimerIntClear(TIMER3_BASE, getTimerIntStatus);

    /* Reconfigure the transfer parameters and re-enable the channel */
    MAP_uDMAChannelTransferSet(UDMA_CH3_TIMER3B | UDMA_PRI_SELECT,
                               UDMA_MODE_AUTO,
                               srcBuffer, dstBuffer, sizeof(srcBuffer));
    MAP_uDMAChannelEnable(UDMA_CH3_TIMER3B);

    bSetEventFlag = 1;
}

int main(void)
{
    bool setErrorFlag = 0;
    uint32_t ii;
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Configure the GPIO PA7 as Timer-3 CCP1 output */
    MAP_GPIOPinConfigure(GPIO_PA7_T3CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);

    /* Enable the Timer-3 in 16-bit PWM mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3)))
    {
    }

    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);

    /* Set the Timer3B load value to systemClock/2000 for a 2 KHz output PWM
     * To get a 66% duty cycle, configure the Match Value as 1/3 of the load
     * value. The timer shall count from load value to match value and the
     * output will be high. From the match value to the 0 the timer will be
     * low. */
    MAP_TimerLoadSet(TIMER3_BASE, TIMER_B, systemClock/2000);

    MAP_TimerMatchSet(TIMER3_BASE, TIMER_B,
                  MAP_TimerLoadGet(TIMER3_BASE, TIMER_B) / 3);

    /* Enable the event generation on the rising edge of the PWM output*/
    MAP_TimerControlEvent(TIMER3_BASE, TIMER_B, TIMER_EVENT_POS_EDGE);

    /* Enable the DMA Done Interrupt bit from the Timer */
    MAP_TimerIntEnable(TIMER3_BASE, TIMER_TIMB_DMA);

    MAP_TimerDMAEventSet(TIMER3_BASE, TIMER_DMA_CAPEVENT_B);

    /* Enable the DMA and Configure Channel for TIMER3B for basic mode of
     * transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the Timer 3B DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH3_TIMER3B);

    /* Put the attributes in a known state for the uDMA TIMER3B channel. These
     * should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH3_TIMER3B,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the control structure for the
     * TIMER3B channel. The primary control structure is used for copying the
     * data from srcBuffer to dstBuffer. The transfer data size is 32 bits and
     * the source & destination address is incremented by 4-bytes. The
     * arbitration size is set to 4.
     */
    MAP_uDMAChannelControlSet(UDMA_CH3_TIMER3B | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_32 | UDMA_DST_INC_32 |
                              UDMA_ARB_4);

    /* Set up the transfer parameters for the TIMER control structure. The mode
     * is Auto mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH3_TIMER3B | UDMA_PRI_SELECT,
                               UDMA_MODE_AUTO,
                               srcBuffer, dstBuffer, sizeof(srcBuffer));

    /* The uDMA TIMER channel is primed to start a transfer. As soon as the
     * channel is enabled and the Timer will issue a DMA Request, the data
     * transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH3_TIMER3B);

    /* Enable the timer interrupt */
    MAP_IntEnable(INT_TIMER3B);

    /* Enable the timer */
    MAP_TimerEnable(TIMER3_BASE, TIMER_B);

    while(1)
    {
        /* Wait for an event capture and clear the flag */
        while(!(bSetEventFlag))
        {

        }

        bSetEventFlag = 0;

        /* Check the data transfer by comparing the source and destination
         * buffers. If there is a mismatch then print the same and break
         * the loop. Also clear the destination buffer if the data is a
         * match */
        for(ii = 0; ii < MEM_BUF_SIZE; ii++)
        {
            if(srcBuffer[ii] != dstBuffer[ii])
            {
                setErrorFlag = 1;
            }
            else
            {
                dstBuffer[ii] = 0x0;
            }
        }

        if(setErrorFlag)
        {
            break;
        }
    }
}
