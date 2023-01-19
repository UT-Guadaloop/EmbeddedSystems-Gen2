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
 * MSP432E4 Example project for ADC sampling using phase shift.
 *
 * Description: In this application example the ADC0 and ADC1 are configured
 * to sample a single channel with a phase shift of 180. The DMA is used to
 * copy data from each of the ADC to the memory.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|               PE3|<-- AIN0
 *          | |                  |
 *          --|RST               |
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
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

static uint16_t dataBufferA[1];
static uint16_t dataBufferB[1];

volatile bool bgetConvStatus = false;;

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

void ADC0SS3_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* If the interrupt status for Sequencer-3 is set the
     * clear the status and read the data */
    if((getIntStatus & ADC_INT_DMA_SS3) == ADC_INT_DMA_SS3)
    {
        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS3);

        /* Reconfigure the channel control structure and enable the channel */
        MAP_uDMAChannelTransferSet(UDMA_CH17_ADC0_3 | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)&ADC0->SSFIFO3, (void *)&dataBufferA,
                                   sizeof(dataBufferA)/2);

        MAP_uDMAChannelEnable(UDMA_CH17_ADC0_3);
    }
}

void ADC1SS3_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC1_BASE, true);

    /* If the interrupt status for Sequencer-3 is set the
     * clear the status and read the data */
    if((getIntStatus & ADC_INT_DMA_SS3) == ADC_INT_DMA_SS3)
    {
        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClearEx(ADC1_BASE, ADC_INT_DMA_SS3);

        /* Reconfigure the channel control structure and enable the channel */
        MAP_uDMAChannelTransferSet(UDMA_CH27_ADC1_3 | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)&ADC1->SSFIFO3, (void *)&dataBufferB,
                                   sizeof(dataBufferB)/2);

        MAP_uDMAChannelEnable(UDMA_CH27_ADC1_3);

        /* Set conversion flag to true */
        bgetConvStatus = true;
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

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }

    /* Configure PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    /* Enable the clock to ADC-1 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1)))
    {
    }

    /* Configure Sequencer 3 to sample the analog channel : AIN0. The
     * end of conversion and interrupt generation is set for AIN0 */
    MAP_ADCSequenceStepConfigure(ADC1_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 3 with a Processor signal trigger.  Sequencer 3
     * will do a single sample.*/
    MAP_ADCSequenceConfigure(ADC1_BASE, 3, ADC_TRIGGER_PROCESSOR, 3);

    /* Clear the interrupt status flag before enabling. This is done to make
     * sure the interrupt flag is cleared before we sample. */
    MAP_ADCIntClearEx(ADC1_BASE, ADC_INT_DMA_SS3);
    MAP_ADCIntEnableEx(ADC1_BASE, ADC_INT_DMA_SS3);

    /* Enable the DMA request from ADC0 Sequencer 3 */
    MAP_ADCSequenceDMAEnable(ADC1_BASE, 3);

    /* Set Phase Delay on the channel */
    MAP_ADCPhaseDelaySet(ADC1_BASE, ADC_PHASE_180);

    /* Since sample sequence 32 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC1_BASE, 3);

    /* Enable the Interrupt generation from the ADC-0 Sequencer */
    MAP_IntEnable(INT_ADC1SS3);

    /* Enable the clock to ADC-1 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 3 to sample the analog channel : AIN0. The
     * end of conversion and interrupt generation is set for AIN0 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 3 with a Processor signal trigger.  Sequencer 3
     * will do a single sample*/
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 3);

    /* Clear the interrupt status flag before enabling. This is done to make
     * sure the interrupt flag is cleared before we sample. */
    MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS3);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS3);

    /* Enable the DMA request from ADC0 Sequencer 3 */
    MAP_ADCSequenceDMAEnable(ADC0_BASE, 3);

    /* Since sample sequence 32 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 3);

    /* Enable the Interrupt generation from the ADC-0 Sequencer */
    MAP_IntEnable(INT_ADC0SS3);

    /* Enable the DMA and Configure Channel for ADC0 and ADC1 sequencer for
     * Basic mode of transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the ADC0 Sequencer 3 DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH17_ADC0_3);

    /* Put the attributes in a known state for the uDMA ADC0 Sequencer 3
     * channel. These should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH17_ADC0_3,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the ADC0 Sequencer 3 channel. The primary control structure is used for
     * copying the data from ADC0 Sequencer 3 FIFO to dataBuffer. The transfer
     * data size is 16 bits and the source address is not incremented while
     * the destination address is incremented at 16-bit boundary.
     */
    MAP_uDMAChannelControlSet(UDMA_CH17_ADC0_3 | UDMA_PRI_SELECT,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the ADC0 Sequencer 2 primary control
     * structure. The mode is Basic mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH17_ADC0_3 | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *)&ADC0->SSFIFO3, (void *)&dataBufferA,
                               sizeof(dataBufferA)/2);

    /* The uDMA ADC0 Sequencer 3 channel is primed to start a transfer. As
     * soon as the channel is enabled and the processor issues an ADC trigger,
     * the ADC will perform the conversion and send a DMA Request. The data
     * transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH17_ADC0_3);

    /* Map the ADC1 Sequencer 3 DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH27_ADC1_3);

    /* Put the attributes in a known state for the uDMA ADC1 Sequencer 3
     * channel. These should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH27_ADC1_3,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the ADC1 Sequencer 3 channel. The primary control structure is used for
     * copying the data from ADC1 Sequencer 3 FIFO to dataBuffer. The transfer
     * data size is 16 bits and the source address is not incremented while
     * the destination address is incremented at 16-bit boundary.
     */
    MAP_uDMAChannelControlSet(UDMA_CH27_ADC1_3 | UDMA_PRI_SELECT,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 |
                              UDMA_ARB_1);

    /* Set up the transfer parameters for the ADC0 Sequencer 2 primary control
     * structure. The mode is Basic mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH27_ADC1_3 | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                               (void *)&ADC1->SSFIFO3, (void *)&dataBufferB,
                               sizeof(dataBufferB)/2);

    /* The uDMA ADC0 Sequencer 3 channel is primed to start a transfer. As
     * soon as the channel is enabled and the processor issues an ADC trigger,
     * the ADC will perform the conversion and send a DMA Request. The data
     * transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH27_ADC1_3);

    /* Wait loop */
    while(1)
    {
        MAP_ADCProcessorTrigger(ADC1_BASE, (ADC_TRIGGER_WAIT | 3));
        MAP_ADCProcessorTrigger(ADC0_BASE, (ADC_TRIGGER_SIGNAL | 3));

        /* Wait for the conversion to complete */
        while(!bgetConvStatus);
        bgetConvStatus = false;

        /* Display the digital value on the console. */
        UARTprintf("Data = %4d %4d\r", dataBufferA[0], dataBufferB[0]);
    }
}
