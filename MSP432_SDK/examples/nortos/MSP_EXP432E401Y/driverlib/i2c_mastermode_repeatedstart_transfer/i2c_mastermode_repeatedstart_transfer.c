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
 * MSP432E4 Example Project for I2C-Master for Simple Write and Read with
 * repeated start.
 *
 * Description: This application example configures the I2C module for master
 * mode operation with standard speed. The use of the example requires another
 * MSP-EXP432E401Y board to be running i2c_slavemode_repeatedstart_transfer
 * application. The master board sends a data to the slave and the slave
 * inverts the bits. The master board addresses the byte it wants to read from
 * the slave device to which the slave sends the specific data byte. The master
 * compares the data byte read from the slave with the inverted master data.
 * If there is an error in transmission the LED D2 is switched ON.
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

/* Defines for I2C bus parameters */
#define SLAVE_ADDRESS   0x26
#define I2C_NUM_DATA    5

/* Defines for I2C State Machine */
#define I2C_MASTER_IDLE 0x0
#define I2C_MASTER_TX   0x1
#define I2C_MASTER_RX   0x2
#define I2C_MASTER_ANAK 0x3
#define I2C_MASTER_DNAK 0x4
#define I2C_MASTER_ALST 0x5
#define I2C_MASTER_UNKN 0x6

/* Variables for I2C data and state machine */
uint8_t sendData[I2C_NUM_DATA] = {0x04, 0xA5, 0x36, 0x67, 0x44};
uint8_t getData = 0x00;
uint8_t setI2CState;
uint8_t dataIndex;

void I2C1_IRQHandler(void)
{
    uint32_t getIntStatus;
    uint32_t getERRStatus;

    /* Get the interrupt status and clear the same */
    getIntStatus = MAP_I2CMasterIntStatusEx(I2C1_BASE, true);
    MAP_I2CMasterIntClearEx(I2C1_BASE, getIntStatus);

    /* Check if we have a Data Request */
    if((getIntStatus & I2C_MASTER_INT_DATA) == I2C_MASTER_INT_DATA)
    {
        /* Process data interrupt  for the Transmit Path */
        if((setI2CState == I2C_MASTER_TX) && (dataIndex < I2C_NUM_DATA-1))
        {
            MAP_I2CMasterDataPut(I2C1_BASE, sendData[dataIndex++]);
            MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        else if((setI2CState == I2C_MASTER_TX) && (dataIndex == I2C_NUM_DATA-1))
        {
            MAP_I2CMasterDataPut(I2C1_BASE, sendData[dataIndex++]);
            MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        }

        /* Process data interrupt  for the Receive Path */
        if(setI2CState == I2C_MASTER_RX)
        {
            MAP_I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS, true);
            MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        }
    }

    /* Check if we have a Stop condition on the bus */
    if((getIntStatus & I2C_MASTER_INT_STOP) == I2C_MASTER_INT_STOP)
    {
        if(setI2CState == I2C_MASTER_TX)
        {
            setI2CState = I2C_MASTER_IDLE;
        }
        else if(setI2CState == I2C_MASTER_RX)
        {
            getData = MAP_I2CMasterDataGet(I2C1_BASE);
            setI2CState = I2C_MASTER_IDLE;
        }
    }

    /* Check if we have an ADDR NAK, DATA NAK or ARB LOST condition on the I2C
     * bus */
    if((getIntStatus & I2C_MASTER_INT_NACK) == I2C_MASTER_INT_NACK)
    {
        /* Set the Error LED */
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);

        getERRStatus = MAP_I2CMasterErr(I2C1_BASE);
        if((getERRStatus & I2C_MASTER_ERR_ADDR_ACK) == I2C_MASTER_ERR_ADDR_ACK)
        {
            setI2CState = I2C_MASTER_ANAK;
        }

        if((getERRStatus & I2C_MASTER_ERR_DATA_ACK) == I2C_MASTER_ERR_DATA_ACK)
        {
            setI2CState = I2C_MASTER_DNAK;
        }

        if((getERRStatus & I2C_MASTER_ERR_ARB_LOST) == I2C_MASTER_ERR_ARB_LOST)
        {
            setI2CState = I2C_MASTER_ALST;
        }
    }
}

int main(void)
{
    uint8_t  ii;
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

    /* Configure the I2C Master in standard mode and enable interrupt for Data
     * completion, NAK and Stop condition on the bus */
    MAP_I2CMasterInitExpClk(I2C1_BASE, systemClock, false);
    MAP_I2CMasterIntEnableEx(I2C1_BASE, I2C_MASTER_INT_NACK |
                                        I2C_MASTER_INT_STOP |
                                        I2C_MASTER_INT_DATA);

    /* Initialize the state of the I2C Master */
    setI2CState = I2C_MASTER_IDLE;

    /* Enable the interrupt generation from I2C-1 */
    MAP_IntEnable(INT_I2C1);

    while(1)
    {
        /* Initialize the variables for Tx */
        setI2CState = I2C_MASTER_TX;
        dataIndex   = 0;

        /* Put the Slave Address on the bus for Write */
        MAP_I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS, false);

        /* Write the first data to the bus */
        MAP_I2CMasterDataPut(I2C1_BASE, sendData[dataIndex++]);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        /* Wait for all the bytes to be sent */
        while(setI2CState == I2C_MASTER_TX)
        {
        }

        /* Put a Short Delay Loop */
        MAP_SysCtlDelay(1000);

        /* Now read each of the 4 bytes one after the other by using repeated
         * start. The Master shall place a Write request to the slave with
         * the byte number of the data and then perform a read transaction to
         * get the data from the slave */
        for(ii = 0; ii < (I2C_NUM_DATA-1); ii++)
        {
            /* Initialize the variables for Rx */
            setI2CState = I2C_MASTER_RX;
            dataIndex   = 0;

            /* Put the Slave Address on the bus for Write */
            MAP_I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS, false);

            /* Put the Byte Number to read back and send Start Command */
            MAP_I2CMasterDataPut(I2C1_BASE, ii);
            MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

            /* Wait for all the bytes to be received */
            while(setI2CState == I2C_MASTER_RX)
            {
            }

            if(sendData[ii+1] != (getData ^ 0xFF))
            {
                MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
            }
            else
            {
                getData = 0x0;
            }
        }
    }
}
