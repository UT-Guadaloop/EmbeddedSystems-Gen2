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
 * MSP432E4 Example Project for I2C-Master for Burst Write and Read using I2C
 * FIFO and CPU.
 *
 * Description: This application example configures the I2C module for master
 * mode operation with standard speed. The use of the example requires another
 * MSP-EXP432E401Y board to be running i2c_slavemode_masterfifo_transfer
 * application.
 * The master board sends a 32 bytes to the slave using the FIFO. The master
 * board addresses the byte offset it wants to read from the slave device to
 * which the slave sends the data back to the master. The master uses the FIFO
 * to receive the data from the Slave. The master compares the data byte
 * stream read from the slave. If there is an error in transmission the LED D2
 * is switched ON.
 *
 *                MSP432E401Y                      MSP432E401Y
 *             ------------------               ------------------
 *         /|\|      MASTER      |             |      SLAVE       |
 *          | |                  |             |                  |
 *          --|RST            PG0|<->I2C1SCL<->|PG0               |
 *            |               PG1|<->I2C1SDA<->|PG1               |
 *            |                  |             |                  |
 *            |               PN0|-->LED D2    |                  |
 *            |                  |             |                  |
 *            |                  |             |                  |
 * Author: 
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* Defines for I2C bus parameters */
#define SLAVE_ADDRESS   0x26
#define I2C_NUM_DATA    32

/* Enumerated Data Types for I2C State Machine */
enum I2C_MASTER_STATE
{
    I2C_OP_IDLE = 0,
    I2C_OP_FIFO,
    I2C_OP_TXDATA,
    I2C_OP_RXDATA,
    I2C_OP_STOP,
    I2C_ERR_STATE
};

/* Variables for I2C data and state machine */
uint8_t  sendMasterTxData[I2C_NUM_DATA];
uint8_t           getMasterRxData[I2C_NUM_DATA];
uint8_t  setMasterCurrState;
uint8_t  setMasterPrevState;
bool     setI2CDirection;
bool     setI2CRepeatedStart;
uint8_t  setMasterBytes       = I2C_NUM_DATA;
const    uint8_t  setMasterBytesLength = I2C_NUM_DATA;
uint8_t  setTxBufferAvail;
uint8_t  setRxBufferAvail;
uint8_t  setReadPointerStart;

void I2C1_IRQHandler(void)
{
    uint32_t getInterruptStatus;
    uint8_t  ii;

    /* Get the masked interrupt status */
    getInterruptStatus = MAP_I2CMasterIntStatusEx(I2C1_BASE, true);

    /* Execute the State Machine */
    switch (setMasterCurrState) {
    case I2C_OP_IDLE:
        /* Move from IDLE to Transmit Address State */
        setMasterPrevState = setMasterCurrState;
        setMasterCurrState = I2C_OP_FIFO;

        /* Write the transfer length to the Slave during write. When reading
         * the data from the slave send the start address of the bytes to be
         * read back */
        MAP_I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS, false);
        if(!setI2CDirection)
        {
            MAP_I2CMasterDataPut(I2C1_BASE, (I2C_NUM_DATA-1));
        }
        else
        {
            MAP_I2CMasterDataPut(I2C1_BASE, setReadPointerStart);
        }
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        break;

    case I2C_OP_FIFO:
        /* If Last Data has been NAK'ed then go to stop state */
        if(getInterruptStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        /* Based on the direction move to the appropriate state of Transmit or
         * Receive. Also send the BURST command for FIFO Operations. */
        else if(!setI2CDirection)
        {
            setMasterCurrState = I2C_OP_TXDATA;
            MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_FIFO_BURST_SEND_FINISH);
        }
        else
        {
            setMasterCurrState = I2C_OP_RXDATA;
            MAP_I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS, true);
            MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE);
        }
        break;

    case I2C_OP_TXDATA:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte */
        setMasterPrevState = setMasterCurrState;

        /* If Address or Data has been NAK'ed then go to stop state. If a Stop
         * condition is seen due to number of bytes getting done then move to
         * STOP state */
        if(getInterruptStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getInterruptStatus & I2C_MASTER_INT_STOP)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getInterruptStatus & I2C_MASTER_INT_TX_FIFO_REQ)
        {
            if(setMasterBytes <= (setMasterBytesLength - setTxBufferAvail))
            {
                for(ii = 0; ii < setTxBufferAvail ; ii++)
                {
                    MAP_I2CFIFODataPutNonBlocking(I2C1_BASE,
                                                  sendMasterTxData[setMasterBytes++]);
                }
            }
            else
            {
                setTxBufferAvail = setMasterBytesLength - setMasterBytes;
                for(ii = 0; ii < setTxBufferAvail; ii++)
                {
                    MAP_I2CFIFODataPutNonBlocking(I2C1_BASE,
                                                  sendMasterTxData[setMasterBytes++]);
                }
            }
        }
        else
        {
            setMasterCurrState = I2C_ERR_STATE;
        }
        break;

    case I2C_OP_RXDATA:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte */
        setMasterPrevState = setMasterCurrState;

        /* If Address has been NAK'ed then go to stop state. If a Stop
         * condition is seen due to number of bytes getting done then move to
         * STOP state and read the last data byte */
        if(getInterruptStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getInterruptStatus & I2C_MASTER_INT_STOP)
        {
            setMasterCurrState = I2C_OP_STOP;

            if((MAP_I2CFIFOStatus(I2C1_BASE) & I2C_FIFO_RX_EMPTY) != I2C_FIFO_RX_EMPTY)
            {
                MAP_I2CFIFODataGetNonBlocking(I2C1_BASE,
                                              &getMasterRxData[setMasterBytes++]);
            }
        }
        else if(getInterruptStatus & I2C_MASTER_INT_RX_FIFO_REQ)
        {
            if(setMasterBytes <= (setMasterBytesLength - setRxBufferAvail))
            {
                for(ii = 0; ii < setRxBufferAvail ; ii++)
                {
                    MAP_I2CFIFODataGetNonBlocking(I2C1_BASE,
                                                  &getMasterRxData[setMasterBytes++]);
                }
            }
            else
            {
                setRxBufferAvail = setMasterBytesLength - setMasterBytes;
                for(ii = 0; ii < setRxBufferAvail; ii++)
                {
                    MAP_I2CFIFODataGetNonBlocking(I2C1_BASE,
                                                  &getMasterRxData[setMasterBytes++]);
                }
            }
        }
        else
        {
            setMasterCurrState = I2C_ERR_STATE;
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

    /* Clear the Interrupt Status flags */
    MAP_I2CMasterIntClearEx(I2C1_BASE, getInterruptStatus);
}

int main(void)
{
    uint8_t  ii;
    uint8_t  setTemp;
    uint32_t systemClock;

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    /* Enable clocks to GPIO Port N and configure pins as Output */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)))
    {
    }
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

    /* Enable clocks to GPIO Port G and configure pins as I2C */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG)))
    {
    }

    MAP_GPIOPinConfigure(GPIO_PG0_I2C1SCL);
    MAP_GPIOPinConfigure(GPIO_PG1_I2C1SDA);
    MAP_GPIOPinTypeI2C(GPIO_PORTG_BASE, GPIO_PIN_1);
    MAP_GPIOPinTypeI2CSCL(GPIO_PORTG_BASE, GPIO_PIN_0);

    /* Since there are no board pull up's we shall enable the weak internal
     * pull up */
    GPIOG->PUR |= (GPIO_PIN_1 | GPIO_PIN_0);

    /* Enable the clock to I2C-1 module and configure the I2C Master */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1)))
    {
    }

    /* Configure the I2C Master in standard mode and enable interrupt for Rx
     * FIFO Request, TX FIFO Request, Arbitration Lost, Stop, NAK, Timeout and
     * Data completion condition on the bus */
    MAP_I2CMasterInitExpClk(I2C1_BASE, systemClock, false);
    MAP_I2CMasterIntEnableEx(I2C1_BASE, (I2C_MASTER_INT_RX_FIFO_REQ |
                                         I2C_MASTER_INT_TX_FIFO_REQ |
                                         I2C_MASTER_INT_ARB_LOST |
                                         I2C_MASTER_INT_STOP |
                                         I2C_MASTER_INT_NACK |
                                         I2C_MASTER_INT_TIMEOUT |
                                         I2C_MASTER_INT_DATA));

    /* Assign the Transmit and Receive FIFO to the Master Transmit threshold
     * of 2 means that when there are less than or equal to 2 bytes in the TX
     * FIFO then generate an interrupt.
     * Receive threshold of 6 means that when there are more than or equal to
     * 6 bytes in the RX FIFO then generate an interrupt. */
    MAP_I2CTxFIFOConfigSet(I2C1_BASE, I2C_FIFO_CFG_TX_MASTER |
                                      I2C_FIFO_CFG_TX_TRIG_4);
    MAP_I2CRxFIFOConfigSet(I2C1_BASE, I2C_FIFO_CFG_RX_MASTER |
                                      I2C_FIFO_CFG_RX_TRIG_6);

    /* Flush any existing data in the FIFO */
    MAP_I2CTxFIFOFlush(I2C1_BASE);
    MAP_I2CRxFIFOFlush(I2C1_BASE);

    /* Enable the Glitch Filter */
    MAP_I2CMasterGlitchFilterConfigSet(I2C1_BASE, I2C_MASTER_GLITCH_FILTER_8);

    /* Enable the interrupt generation from I2C-1 */
    MAP_IntEnable(INT_I2C1);

    /* Initialize the Master Transmit Buffer with Random Data and clear the
     * Master Receive Buffer */
    for(ii = 0; ii < I2C_NUM_DATA; ii++)
    {
        sendMasterTxData[ii] = rand() & 0xFF;
        getMasterRxData[ii]  = 0x0;
    }


    /* Initialize the state of the I2C Master */
    setMasterCurrState = I2C_OP_IDLE;
    MAP_I2CMasterBurstLengthSet(I2C1_BASE, setMasterBytesLength);

    /* Set Transmit Flag */
    setI2CDirection     = false;
    setMasterBytes      = 0;
    setTxBufferAvail    = 4;

    /* Trigger the Transfer using Software Interrupt */
    MAP_IntTrigger(INT_I2C1);
    while(setMasterCurrState != I2C_OP_STOP);
    setMasterCurrState = I2C_OP_IDLE;

    /* Set the I2CMBLEN register and also initialize */
    MAP_I2CMasterBurstLengthSet(I2C1_BASE, setMasterBytesLength);

    /* Set receive Flag with Repeated Start. Also Receive Buffer is calculated
     *  as the Trigger Level to read back data. */
    setI2CDirection        = true;
    setI2CRepeatedStart    = true;
    setMasterBytes         = 0;
    setReadPointerStart    = 4;
    setRxBufferAvail       = 6;

    /* Trigger the Transfer using Software Interrupt */
    MAP_IntTrigger(INT_I2C1);
    while(setMasterCurrState != I2C_OP_STOP);
    setMasterCurrState = I2C_OP_IDLE;

    /* Perform Data Integrity Check here */
    setTemp = (I2C_NUM_DATA - 1) - setReadPointerStart;

    for(ii = 0; ii < I2C_NUM_DATA; ii++)
    {
        /* Adjust the temp pointer to the read buffer to implement a circular
         * read buffer */
        if(setTemp == (I2C_NUM_DATA-1))
        {
            setTemp = 0;
        }
        else
        {
            setTemp++;
        }

        if(sendMasterTxData[ii] != getMasterRxData[setTemp])
        {
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
        }
    }

    while(1);
}
