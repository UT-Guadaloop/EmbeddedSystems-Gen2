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
 * MSP432E4 I2C Master project for performing data transfer with DMA and
 * I2CFIFODATA register.
 *
 * Description: The use of this application requires an external memory board
 * as shown in the application note SLAA776. I2C-2 instance is used to write
 * and read data from the external memory board using the I2CFIFODATA.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PL1|<-> I2C2 SCL
 *            |               PL0|<-> I2C2 SDA
 *            |                  |
 *            |               PL4|--> GPIO for LA
 *            |                  |
 *            |                  |
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* Display Include via console */
#include "uartstdio.h"

/* Define for I2C Slave Module */
#define SLAVE_ADDRESS_EXT 0x50
#define NUM_OF_I2CBYTES   255

/* Enumerated Data Types for Master State Machine */
enum I2C_MASTER_STATE
{
    I2C_OP_IDLE = 0,
    I2C_OP_TXADDR,
    I2C_OP_FIFO,
    I2C_OP_TXDATA,
    I2C_OP_RXDATA,
    I2C_OP_STOP,
    I2C_ERR_STATE
};

/* Global variables */
volatile uint16_t setSlaveWordAddress;
uint8_t  setMasterTxData[NUM_OF_I2CBYTES];
uint8_t  getMasterRxData[NUM_OF_I2CBYTES];
volatile uint8_t  setMasterCurrState;
volatile uint8_t  setMasterPrevState;
volatile bool     setI2CDirection;
volatile bool     setI2CRepeatedStart;
volatile uint8_t  setMasterBytes       = NUM_OF_I2CBYTES;
volatile uint8_t  setMasterBytesLength = NUM_OF_I2CBYTES;

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

void I2C2_IRQHandler(void)
{
    uint32_t getI2CIntStatus;

    /* Toggle PL4 High to Indicate Entry to ISR */
    MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_PIN_4);

    /* Get the masked interrupt status and clear the flags */
    getI2CIntStatus = MAP_I2CMasterIntStatusEx(I2C2_BASE, true);
    MAP_I2CMasterIntClearEx(I2C2_BASE, getI2CIntStatus);

    /* Execute the State Machine */
    switch (setMasterCurrState) {
    case I2C_OP_IDLE:
        /* Move from IDLE to Transmit Address State */
        setMasterPrevState = setMasterCurrState;
        setMasterCurrState = I2C_OP_TXADDR;

        /* Write the upper bits of the page to the Slave */
        MAP_I2CMasterSlaveAddrSet(I2C2_BASE, SLAVE_ADDRESS_EXT, false);
        MAP_I2CMasterDataPut(I2C2_BASE, (setSlaveWordAddress >> 8));
        MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        break;

    case I2C_OP_TXADDR:
        /* Assign the current state to the previous state */
        setMasterPrevState = setMasterCurrState;

        /* If Address has been NAK'ed then go to stop state else go the FIFO
         * Priming State */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else
        {
            setMasterCurrState = I2C_OP_FIFO;
        }

        /* Write the lower bits of the page to the Slave if address has been
         * ACK-ed */
        MAP_I2CMasterDataPut(I2C2_BASE, (setSlaveWordAddress >> 0));
        MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        break;

    case I2C_OP_FIFO:
        /* If Last Data has been NAK'ed then go to stop state */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        /* Based on the direction move to the appropriate state of Transmit or
         *  Receive. Also send the BURST command for FIFO Operations. */
        else if(!setI2CDirection)
        {
            setMasterCurrState = I2C_OP_TXDATA;
            MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_FIFO_BURST_SEND_FINISH);
        }
        else
        {
            setMasterCurrState = I2C_OP_RXDATA;
            MAP_I2CMasterSlaveAddrSet(I2C2_BASE, SLAVE_ADDRESS_EXT, true);
            MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE);
        }
        break;

    case I2C_OP_TXDATA:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte */
        setMasterPrevState = setMasterCurrState;

        /* If Address or Data has been NAK'ed then go to stop state if a Stop
         * condition is seen due to number of bytes getting done then move to
         * STOP state. */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getI2CIntStatus & I2C_MASTER_INT_STOP)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getI2CIntStatus & I2C_MASTER_INT_TX_DMA_DONE)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else
        {
            setMasterCurrState = I2C_ERR_STATE;
        }
        break;

    case I2C_OP_RXDATA:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte. */
        setMasterPrevState = setMasterCurrState;

        /* If Address has been NAK'ed then go to stop state if a Stop
         * condition is seen due to number of bytes getting done then move to
         * STOP state and read the last data byte */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getI2CIntStatus & I2C_MASTER_INT_STOP)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getI2CIntStatus & I2C_MASTER_INT_RX_DMA_DONE)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        break;

    case I2C_OP_STOP:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte */
        setMasterPrevState = setMasterCurrState;
        break;

    case I2C_ERR_STATE:
        setMasterCurrState = I2C_ERR_STATE;
        break;

    default:
        setMasterCurrState = I2C_ERR_STATE;
        break;
    }

    /* Toggle PL4 Low to Indicate Exit from ISR */
    MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, 0x0);
}

/* This function sets up UDMA Channel Control Structures for TX */
void
ConfigureuDMATX(uint8_t ui8Length, uint32_t ui32ArbSize)
{
    /* Put the attributes in a known state for the uDMA channel. These should
     * already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH5_I2C2TX,
                                    UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the I2C2 TX channel.  The channel
     * will be used to transfer between memory buffer and I2CFIFODATA register
     * 8 bits at a time. Therefore the data size is 8 bits, and the address
     * increment is 8 bits for source. The destination increment is none. The
     * arbitration size will be set based on the TX FIFO Threshold, which
     * causes the uDMA controller to rearbitrate after N items are transferred.
     * This keeps this channel from hogging the uDMA controller once the
     * transfer is started and allows other channels cycles if they are higher
     * priority. */
    MAP_uDMAChannelControlSet(UDMA_CH5_I2C2TX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_8 |
                              UDMA_DST_INC_NONE |  ui32ArbSize);

    /* Set up the transfer parameters for the hardware channel.  This will
     * configure the transfer buffers and the transfer size.  Basic mode is
     * used in this example */
    MAP_uDMAChannelTransferSet(UDMA_CH5_I2C2TX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC, (void *)&setMasterTxData,
                               (void *)&I2C2->FIFODATA,
                               ui8Length);

    /* The channel must be enabled.  For hardware based transfers, a request
     * must be issued by the peripheral.  After this, the uDMA memory transfer
     * begins. */
    MAP_uDMAChannelEnable(UDMA_CH5_I2C2TX);

}

/* This function sets up UDMA Channel Control Structures for RX */
void
ConfigureuDMARX(uint8_t ui8Length, uint32_t ui32ArbSize)
{
    /* Put the attributes in a known state for the uDMA software channel.
     * These should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH4_I2C2RX,
                                    UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the I2C2 RX channel.  The channel
     * will be used to transfer between I2CFIFODATA register and memory
     * buffers, 8 bits at a time. Therefore the data size is 8 bits, and the
     * address increment is 8 bits for destination. The source increment is
     * none. The arbitration size will be set based on the RX FIFO Threshold,
     * which causes the uDMA controller to rearbitrate after N items are
     * transferred. This keeps this channel from hogging the uDMA controller
     * once the transfer is started and allows other channels cycles if they
     * are higher priority. */
    MAP_uDMAChannelControlSet(UDMA_CH4_I2C2RX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_NONE |
                              UDMA_DST_INC_8 | ui32ArbSize);

    /* Set up the transfer parameters for the hardware channel.  This will
     * configure the transfer buffers and the transfer size.  Basic mode is
     * used in this example */
    MAP_uDMAChannelTransferSet(UDMA_CH4_I2C2RX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC, (void *)&I2C2->FIFODATA,
                               (void *)&getMasterRxData,
                               ui8Length);

    /* The channel must be enabled.  For hardware based transfers, a request
     * must be issued by the peripheral.  After this, the uDMA memory transfer
     * begins. */
    MAP_uDMAChannelEnable(UDMA_CH4_I2C2RX);

}

void
InitConsole(void)
{
    /* Enable GPIO port A which is used for UART0 pins. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Enable UART-0 for serial output */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the pin muxing for UART0 functions on port A0 and A1. This
     * step is not necessary if the part does not support pin muxing. */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);

    /* Select the alternate (UART) function for these pins. */
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Use the internal 16MHz oscillator as the UART clock source. */
    MAP_UARTClockSourceSet(UART0_BASE, UART_CLOCK_ALTCLK);

    /* Initialize the UART for console I/O. */
    UARTStdioConfig(0, 115200, 16000000);
}

int main(void)
{
    uint32_t getSystemClock;
    uint8_t  ii, setTransferLength;
    uint32_t setTxArbSize, setRxArbSize;
    bool     setErrorStatus;

    /* Set up the serial console to use for displaying messages.  This is just
     * for this example program. */
    InitConsole();

    /* Display the setup on the console. */
    UARTprintf("\033[2J\033[H");
    UARTprintf("\r\nExample Code for I2C Master with");
    UARTprintf("\nUDMA and FIFO Data Transfer\n\n");

    /* Stop the Clock, Reset and Enable UDMA Module in Master Function */
    MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_UDMA);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_UDMA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);

    /* Wait for the Peripheral to be ready for programming */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA));

    /* Enable the uDMA controller. */
    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Assign the UDMA Channel for I2C2 RX and TX DMA Request */
    MAP_uDMAChannelAssign(UDMA_CH4_I2C2RX);
    MAP_uDMAChannelAssign(UDMA_CH5_I2C2TX);

    /* Enable GPIO for Configuring the I2C Interface Pins */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);

    /* Wait for the Peripheral to be ready for programming */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));

    /* Configure Pins for I2C2 Master Interface */
    MAP_GPIOPinConfigure(GPIO_PL1_I2C2SCL);
    MAP_GPIOPinConfigure(GPIO_PL0_I2C2SDA);
    MAP_GPIOPinTypeI2C(GPIO_PORTL_BASE, GPIO_PIN_0);
    MAP_GPIOPinTypeI2CSCL(GPIO_PORTL_BASE, GPIO_PIN_1);

    /* Configure GPIO Pin PL4 for Interrupt Time Processing */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_4);
    MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, 0x0);

    /* Setup System Clock for 120MHz */
    getSystemClock = MAP_SysCtlClockFreqSet((SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_XTAL_25MHZ |
                                             SYSCTL_CFG_VCO_480), 120000000);

    /* Stop the Clock, Reset and Enable I2C Module in Master Function */
    MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C2);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);

    /* Wait for the Peripheral to be ready for programming */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2));

    /* Initialize and Configure the Master Module */
    MAP_I2CMasterInitExpClk(I2C2_BASE, getSystemClock, true);

    /* Assign the Transmit and Receive FIFO to the Master Transmit threshold
     * of 2 means that when there are less than or equal to 2 bytes in the TX
     * FIFO then generate an interrupt. Receive threshold of 6 means that when
     * there are more than or equal to 6 bytes in the RX FIFO then generate an
     * interrupt */
    MAP_I2CTxFIFOConfigSet(I2C2_BASE, I2C_FIFO_CFG_TX_MASTER_DMA |
                                      I2C_FIFO_CFG_TX_TRIG_2);
    MAP_I2CRxFIFOConfigSet(I2C2_BASE, I2C_FIFO_CFG_RX_MASTER_DMA |
                                      I2C_FIFO_CFG_RX_TRIG_3);

    /* Transmit uDMA Arbitrartion size is calculated as the smaller mod 2
     * which can write to the TXFIFO without overrun
     * TX TRIGGER               ARB
     * I2C_FIFO_CFG_TX_NO_TRIG  UDMA_ARB_8
     * I2C_FIFO_CFG_TX_TRIG_1   UDMA_ARB_4
     * I2C_FIFO_CFG_TX_TRIG_2   UDMA_ARB_4
     * I2C_FIFO_CFG_TX_TRIG_3   UDMA_ARB_4
     * I2C_FIFO_CFG_TX_TRIG_4   UDMA_ARB_4
     * I2C_FIFO_CFG_TX_TRIG_5   UDMA_ARB_2
     * I2C_FIFO_CFG_TX_TRIG_6   UDMA_ARB_2
     * I2C_FIFO_CFG_TX_TRIG_7   UDMA_ARB_1 */
    setTxArbSize = UDMA_ARB_4;

    /* Receive uDMA Arbitrartion size is calculated as the trigger level to
     * read back data from RXFIFO without an underrun.
     * RX TRIGGER               ARB
     * I2C_FIFO_CFG_RX_NO_TRIG  INVALID
     * I2C_FIFO_CFG_RX_TRIG_1   UDMA_ARB_1
     * I2C_FIFO_CFG_RX_TRIG_2   UDMA_ARB_2
     * I2C_FIFO_CFG_RX_TRIG_3   UDMA_ARB_2
     * I2C_FIFO_CFG_RX_TRIG_4   UDMA_ARB_4
     * I2C_FIFO_CFG_RX_TRIG_5   UDMA_ARB_4
     * I2C_FIFO_CFG_RX_TRIG_6   UDMA_ARB_4
     * I2C_FIFO_CFG_RX_TRIG_7   UDMA_ARB_4 */
    setRxArbSize = UDMA_ARB_2;

    /* Flush any existing data in the FIFO */
    MAP_I2CTxFIFOFlush(I2C2_BASE);
    MAP_I2CRxFIFOFlush(I2C2_BASE);

    /* Enable Interrupts for Arbitration Lost, Stop, NAK, Clock Low Timeout
     * and Data. */
    MAP_I2CMasterIntEnableEx(I2C2_BASE, (I2C_MASTER_INT_ARB_LOST |
                                         I2C_MASTER_INT_STOP |
                                         I2C_MASTER_INT_NACK |
                                         I2C_MASTER_INT_TIMEOUT |
                                         I2C_MASTER_INT_DATA));

    /* Enable the Interrupt in the NVIC from I2C Master */
    MAP_IntEnable(INT_I2C2);

    /* Enable the Glitch Filter. Writing a value 0 will disable the glitch
     * filter.
     * I2C_MASTER_GLITCH_FILTER_DISABLED
     * I2C_MASTER_GLITCH_FILTER_1
     * I2C_MASTER_GLITCH_FILTER_2 : Ideal Value when in HS Mode for 120MHz
     *                              clock
     * I2C_MASTER_GLITCH_FILTER_4
     * I2C_MASTER_GLITCH_FILTER_8 : Ideal Value when in Std, Fast, Fast+ for
     *                              120MHz clock
     * I2C_MASTER_GLITCH_FILTER_16
     * I2C_MASTER_GLITCH_FILTER_32
     */
    MAP_I2CMasterGlitchFilterConfigSet(I2C2_BASE, I2C_MASTER_GLITCH_FILTER_8);

    /* Initialize and Configure the Master Module State Machine */
    setMasterCurrState = I2C_OP_IDLE;

    /* Check if the Bus is Busy or not */
    while(MAP_I2CMasterBusBusy(I2C2_BASE));

    /* Randomly Initialize the Transmit buffer and clear the receive buffer */
    for(ii=0 ; ii < NUM_OF_I2CBYTES ; ii++)
    {
        setMasterTxData[ii]   = rand() & 0xFF;
        getMasterRxData[ii]   = 0xFF;
    }

    /* When sending data to slave make sure RX DMA DONE interrupt bit is
     * masked and TX DMA DONE interrupt bit is unmasked */
    MAP_I2CMasterIntEnableEx(I2C2_BASE, I2C_MASTER_INT_TX_DMA_DONE);
    MAP_I2CMasterIntDisableEx(I2C2_BASE, I2C_MASTER_INT_RX_DMA_DONE);

    /* Set the I2CMBLEN register and also initialize the internal flag */
    setTransferLength = 32;

    MAP_I2CMasterBurstLengthSet(I2C2_BASE, setTransferLength);

    /* Configure the uDMA Control Channel Structure for TX */
    ConfigureuDMATX(setTransferLength, setTxArbSize);

    /* Set Transmit Flag and set the Page Address in external slave to 0x0000 */
    setI2CDirection     = false;
    setSlaveWordAddress = 0x0;
    setMasterBytes      = 0;

    /* Print Message before sending data */
    UARTprintf("Transmit %d bytes to external Slave...\n\n",setTransferLength);

    /* Trigger the Transfer using Software Interrupt */
    setMasterCurrState = I2C_OP_IDLE;
    MAP_IntTrigger(INT_I2C2);
    while(setMasterCurrState != I2C_OP_STOP);

    /* When receiving data from slave make sure TX DMA DONE interrupt bit is
     * masked and RX DMA DONE interrupt bit is unmasked */
    MAP_I2CMasterIntEnableEx(I2C2_BASE, I2C_MASTER_INT_RX_DMA_DONE);
    MAP_I2CMasterIntDisableEx(I2C2_BASE, I2C_MASTER_INT_TX_DMA_DONE);

    /* Set the I2CMBLEN register and also initialize the internal flag */
    setTransferLength = 32;

    MAP_I2CMasterBurstLengthSet(I2C2_BASE, setTransferLength);

    /* Configure the uDMA Control Channel Structure for RX */
    ConfigureuDMARX(setTransferLength,setRxArbSize);

    /* Set receive Flag and set the Page Address in external slave to 0x0000 */
    setI2CDirection     = true;
    setI2CRepeatedStart = true;
    setSlaveWordAddress = 0x0;
    setMasterBytes      = 0;

    /* Print Message before receiving data */
    UARTprintf("Receiving %d bytes from external Slave...\n\n",setTransferLength);

    /* Trigger the Transfer using Software Interrupt */
    setMasterCurrState = I2C_OP_IDLE;
    MAP_IntTrigger(INT_I2C2);
    while(setMasterCurrState != I2C_OP_STOP);

    /* Perform Data Integrity Check... */
    setErrorStatus = false;
    for(ii = 0 ; ii < setTransferLength ; ii++)
    {
        if(getMasterRxData[ii] != setMasterTxData[ii])
        {
            setErrorStatus = true;
        }
    }

    if(setErrorStatus)
    {
        UARTprintf("Failure... Data Mismatch!!!\n");
    }
    else
    {
        UARTprintf("Success... Data Match!!!\n");
    }
    while(1);
}
