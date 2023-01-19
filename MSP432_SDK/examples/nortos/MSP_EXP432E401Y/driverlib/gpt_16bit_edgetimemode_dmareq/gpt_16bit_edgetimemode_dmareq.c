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
 * MSP432E4 Example project for Configuring a timer in 16-bit Edge Time mode
 * with DMA Request
 *
 * Description: In this example, the timer is configured in Event Time mode.
 * The GPIO port J Pull Up is enabled so that a switch SW2 press causes an
 * event to be generated. The timer captures the falling edge and generates
 * a DMA request. The DMA request is used to copy data from one SRAM
 * buffer to another in Basic Mode. The time stamp is captured and printed on
 * the UART console along with the time at which the event occurred.
 *
 *                MSP432E401Y       VDD
 *             ------------------    |
 *         /|\|                  |   /
 *          | |                  |   \
 *          --|RST               |   /
 *            |                  |   |----- SW2
 *            |               PA7|<--Event
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Standard Includes */
#include "uartstdio.h"

#define MEM_BUF_SIZE     16

static uint32_t srcBuffer[MEM_BUF_SIZE] = {0x11111111, 0x22222222, 0x33333333, 0x44444444,
                                           0x55555555, 0x66666666, 0x77777777, 0x88888888,
                                           0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC,
                                           0xDDDDDDDD, 0xEEEEEEEE, 0xFFFFFFFF, 0xCA5ECA5E};
static uint32_t dstBuffer[MEM_BUF_SIZE];

volatile bool bSetEventFlag;

volatile uint16_t getTimerCaptureValue;

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

    getTimerCaptureValue = MAP_TimerValueGet(TIMER3_BASE, TIMER_B);

    bSetEventFlag = 1;
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
    bool setErrorFlag = 0;
    uint32_t ii;
    uint32_t systemClock;

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
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOJ->PUR = GPIO_PIN_1;

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Configure the GPIO PA7 as Timer-3 CCP1 pin */
    MAP_GPIOPinConfigure(GPIO_PA7_T3CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);

    /* Enable the Timer-3 in 16-bit Edge Time mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3)))
    {
    }

    /* Configure the Timer-3B in Edge Time Mode. Load the time to count with a
     * periodicity of 0.5 ms. */
    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME_UP);

    MAP_TimerControlEvent(TIMER3_BASE, TIMER_B, TIMER_EVENT_NEG_EDGE);

    MAP_TimerLoadSet(TIMER3_BASE, TIMER_B, systemClock/2000);

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
            UARTprintf("DMA Transfer has Error for event at %d\n",
                       getTimerCaptureValue);
            break;
        }
        else
        {
            UARTprintf("Success!!! DMA Transfer completed for event at %d\n",
                       getTimerCaptureValue);
        }
    }
}
