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
 * MSP432E4 Example Project for I2C-Slave for Burst Write and Read using I2C
 * FIFO and CPU.
 *
 * Description: This application example configures the I2C module for master
 * mode operation with standard speed. The use of the example requires another
 * MSP-EXP432E401Y board to be running i2c_mastermode_fifocpu_transfer or
 * i2c_mastermode_fifodma_transfer application. The master board sends a 32
 * bytes to the slave using the FIFO. The slave reads the data coming from the
 * master. The master board addresses the byte offset it wants to read from
 * the slave device to which the slave sends the data back to the master. The
 * master compares the data byte stream read from the slave. If there is an
 * error in transmission the LED D2 is switched ON.
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
#define I2C_NUM_DATA    32

/* Variables for I2C data and state machine */
volatile uint8_t getDataBuff[I2C_NUM_DATA];
volatile uint8_t getIndex;
volatile uint8_t dataIndex;

void I2C1_IRQHandler(void)
{
    uint32_t getIntStatus;
    uint32_t getStatus;

    /* Get the interrupt status and clear the same */
    getIntStatus = MAP_I2CSlaveIntStatusEx(I2C1_BASE, true);
    MAP_I2CSlaveIntClearEx(I2C1_BASE, getIntStatus);

    /* Get the Status of the data direction */
    getStatus = MAP_I2CSlaveStatus(I2C1_BASE);

    /* Check if we have a Data Request */
    if((getIntStatus & I2C_SLAVE_INT_DATA) == I2C_SLAVE_INT_DATA)
    {
        /* Process data interrupt for the Transmit Path */
        if((getStatus & I2C_SLAVE_ACT_RREQ_FBR) == I2C_SLAVE_ACT_RREQ_FBR)
        {
            getIndex = MAP_I2CSlaveDataGet(I2C1_BASE);
        }
        else if((getStatus & I2C_SLAVE_ACT_RREQ_FBR) == I2C_SLAVE_ACT_RREQ)
        {
            getDataBuff[dataIndex++] = MAP_I2CSlaveDataGet(I2C1_BASE);
        }

        /* Process data interrupt for the Transmit Path by inverting the data
         * before sending it. */
        if(getStatus == I2C_SLAVE_ACT_TREQ)
        {
            if(getIndex == I2C_NUM_DATA)
            {
                getIndex = 0;
            }
            MAP_I2CSlaveDataPut(I2C1_BASE, getDataBuff[getIndex++]);
        }
    }

    /* Check if we have a Stop condition on the bus */
    if((getIntStatus & I2C_SLAVE_INT_STOP) == I2C_SLAVE_INT_STOP)
    {
        /* Clear the index flag */
        dataIndex = 0;
        getIndex  = 0;
    }
}

int main(void)
{
    /* Configure the system clock for 120 MHz */
    MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

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

    /* Enable the clock to I2C-1 module and configure the I2C Slave */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1)))
    {
    }

    /* Configure the I2C Slave in standard mode and enable interrupt for Data
     * completion and Stop condition on the bus */
    MAP_I2CSlaveEnable(I2C1_BASE);
    MAP_I2CSlaveInit(I2C1_BASE, SLAVE_ADDRESS);
    MAP_I2CSlaveIntEnableEx(I2C1_BASE, I2C_SLAVE_INT_STOP |
                                       I2C_SLAVE_INT_DATA);

    /* Enable the interrupt generation from I2C-1 */
    MAP_IntEnable(INT_I2C1);

    while(1)
    {
    }
}
