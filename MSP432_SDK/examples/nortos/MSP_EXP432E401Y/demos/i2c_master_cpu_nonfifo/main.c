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
 * MSP432E4 I2C Master project for performing data transfer with CPU and
 * I2CMDR register.
 *
 * Description: The use of this application requires an external memory board
 * as shown in the application note SLAA776. I2C-2 instance is used to write
 * and read data from the external memory board using the I2CMDR.
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
    I2C_OP_TXDATA,
    I2C_OP_RXDATA,
    I2C_OP_STOP,
    I2C_ERR_STATE
};

/* Global variables */
uint16_t setSlaveWordAddress;
uint8_t  setMasterTxData[NUM_OF_I2CBYTES];
uint8_t  getMasterRxData[NUM_OF_I2CBYTES];
uint8_t  setMasterCurrState;
uint8_t  setMasterPrevState;
bool     setI2CDirection;
bool     setI2CRepeatedStart;
uint8_t  setMasterBytes       = NUM_OF_I2CBYTES;
uint8_t  setMasterBytesLength = NUM_OF_I2CBYTES;

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

        /* If Address has been NAK'ed then go to stop state */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        /* Based on the direction move to the appropriate state of Transmit or
         * Receive */
        else if(!setI2CDirection)
        {
            setMasterCurrState = I2C_OP_TXDATA;
        }
        else
        {
            setMasterCurrState = I2C_OP_RXDATA;
        }

        /* Write the lower bits of the page to the Slave if Address has been
         * ACK-ed */
        MAP_I2CMasterDataPut(I2C2_BASE, (setSlaveWordAddress >> 0));
        MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

        break;
    case I2C_OP_TXDATA:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte */
        setMasterPrevState = setMasterCurrState;

        /* If Address or Data has been NAK'ed then go to stop state if a Stop
         * condition is seen due to number of bytes getting done then move to
         *  STOP state */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getI2CIntStatus & I2C_MASTER_INT_STOP)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else
        {
            MAP_I2CMasterDataPut(I2C2_BASE, setMasterTxData[setMasterBytes++]);
            if(setMasterBytes == setMasterBytesLength)
            {
                MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
            }
            else
            {
                MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
            }

        }

        break;
    case I2C_OP_RXDATA:
        /* Move the current state to the previous state else continue with the
         * transmission till last byte */
        setMasterPrevState = setMasterCurrState;

        /* If Address has been NAK'ed then go to stop state if a Stop condition
         *  is seen due to number of bytes getting done then move to STOP state
         *  and read the last data byte */
        if(getI2CIntStatus & I2C_MASTER_INT_NACK)
        {
            setMasterCurrState = I2C_OP_STOP;
        }
        else if(getI2CIntStatus & I2C_MASTER_INT_STOP)
        {
            setMasterCurrState = I2C_OP_STOP;
            getMasterRxData[setMasterBytes++] = MAP_I2CMasterDataGet(I2C2_BASE);
        }
        else
        {
            /* If end then NAK the byte and put Stop. Else continue with ACK of
             * the current byte and receive the next byte */
            if(setI2CRepeatedStart)
            {
                /* Send the Slave Address with RnW as Receive. If only byte is
                 * to be received then send START and STOP else send START */
                MAP_I2CMasterSlaveAddrSet(I2C2_BASE, SLAVE_ADDRESS_EXT, true);
                if(setMasterBytesLength == 1)
                {
                    MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
                }
                else
                {
                    MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
                }

                /* Change the Repeated Start Flag to false as the condition is
                 * now to receive data */
                setI2CRepeatedStart = false;
            }
            else if(setMasterBytes == (setMasterBytesLength - 2))
            {
                /* Read the byte from I2C Buffer and decrement the number of
                 * bytes counter to see if end has been reached or not */
                getMasterRxData[setMasterBytes++] = MAP_I2CMasterDataGet(I2C2_BASE);

                /* Put a STOP Condition on the bus */
                MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
            }
            else
            {
                /* Read the byte from I2C Buffer and decrement the number of
                 * bytes counter to see if end has been reached or not */
                getMasterRxData[setMasterBytes++] = MAP_I2CMasterDataGet(I2C2_BASE);

                MAP_I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
            }
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

void InitConsole(void)
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
    uint8_t  ii;
    bool     getErrorStatus;

    /* Set up the serial console to use for displaying messages.  This is just
     * for this example program. */
    InitConsole();

    /* Display the setup on the console. */
    UARTprintf("\033[2J\033[H");
    UARTprintf("\r\nExample Code for I2C Master with");
    UARTprintf("\nInterrupt and non-FIFO Data Transfer\n\n");

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

    /* Enable Interrupts for Arbitration Lost, Stop, NAK, Clock Low Timeout
     * and Data. */
    MAP_I2CMasterIntEnableEx(I2C2_BASE, (I2C_MASTER_INT_ARB_LOST |
            I2C_MASTER_INT_STOP | I2C_MASTER_INT_NACK |
            I2C_MASTER_INT_TIMEOUT | I2C_MASTER_INT_DATA));

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
        getMasterRxData[ii]   = 0x0;
    }

    setMasterBytesLength = 32;

    /* Set Transmit Flag and set the Page Address in external slave to 0x0000 */
    setI2CDirection     = false;
    setSlaveWordAddress = 0x0;
    setMasterBytes      = 0;

    /* Print Message before sending data */
    UARTprintf("Transmit %d bytes to external Slave...\n\n",setMasterBytesLength);

    /* Trigger the Transfer using Software Interrupt */
    MAP_IntTrigger(INT_I2C2);
    while(setMasterCurrState != I2C_OP_STOP);
    setMasterCurrState = I2C_OP_IDLE;

    /* Set receive Flag and set the Page Address in external slave to 0x0000 */
    setI2CDirection     = true;
    setI2CRepeatedStart = true;
    setSlaveWordAddress = 0x0;
    setMasterBytes      = 0;

    /* Print Message before receiving data */
    UARTprintf("Receiving %d bytes from external Slave...\n\n",setMasterBytesLength);

    /* Trigger the Transfer using Software Interrupt */
    MAP_IntTrigger(INT_I2C2);
    while(setMasterCurrState != I2C_OP_STOP);
    setMasterCurrState = I2C_OP_IDLE;

    /* Perform Data Integrity Check... */
    getErrorStatus = false;
    for(ii = 0 ; ii < setMasterBytesLength ; ii++)
    {
        if(getMasterRxData[ii] != setMasterTxData[ii])
        {
            getErrorStatus = true;
        }
    }

    if(getErrorStatus)
    {
        UARTprintf("Failure... Data Mismatch!!!\n");
    }
    else
    {
        UARTprintf("Success... Data Match!!!\n");
    }
    while(1);
}