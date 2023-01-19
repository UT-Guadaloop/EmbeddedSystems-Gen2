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
 * MSP432E4 Example Project for CAN Single Message Receive
 *
 * Description: This example shows the basic setup of CAN in order to receive
 * a single message on the CAN bus.
 *
 * This example expects to receive a CAN message with the ID of 0x100 and length
 * of 8. Depending on the value of byte 0, it will turn ON/OFF LED (PN0). It
 * uses the UART to display the number of messages received and the message
 * itself.
 *
 * Note: Requires CAN transceiver and CAN sniffer tool, or another MSP432E
 *       running the can_singlemessage_transmit example (this example also
 *       requires a CAN transceiver).
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PA0|<-- CAN0RX
 *            |               PA1|--> CAN0TX
 *            |                  |
 *            |                  |
 *            |               PN0|--> LED
 *            |                  |
 *
 * Author: David Lara
*******************************************************************************/
/* DriverLib Includes */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include "uartstdio.h"

/* System clock rate in Hz */
uint32_t sysClock;

/* CAN variables */
bool rxMsg = false;
bool errFlag = false;
uint32_t msgCount = 0;

/* Function prototypes */
void configureUART(void);
void configureCAN(void);

int main(void)
{
    tCANMsgObject sCANMessage;
    uint8_t msgData[8] = {0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00};

    /* Run from the PLL at 120 MHz */
    sysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

    /* Initialize the UART */
    configureUART();

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Enable the GPIO port that is used for the on-board LED */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    /* Check if the peripheral access is enabled */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    /* Turn off the LED */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));

    /* Enable the GPIO pin for the LED (PN0).  Set the direction as output, and
     * enable the GPIO pin for digital function */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    /* Initialize the CAN */
    configureCAN();

    /* Initialize message object 1 to be able to receive CAN message ID 0x100 */
    sCANMessage.ui32MsgID = 0x100;
    sCANMessage.ui32MsgIDMask = 0;

    /* Enable interrupt on RX */
    sCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE;

    /* Size of message is 8 */
    sCANMessage.ui32MsgLen = sizeof(msgData);

    /* Load the message object into the CAN peripheral. Once loaded an
     * interrupt will occur only when the CAN ID is 0x100. Use message
     * object 1 for receiving messages */
    MAP_CANMessageSet(CAN0_BASE, 1, &sCANMessage, MSG_OBJ_TYPE_RX);

    /* Loop forever */
    while(1)
    {
        /* A new message is received */
        if (rxMsg)
        {
            /* Re-use the same message object that was used earlier to configure
             * the CAN */
            sCANMessage.pui8MsgData = (uint8_t *)&msgData;

            /* Read the message from the CAN */
            MAP_CANMessageGet(CAN0_BASE, 1, &sCANMessage, 0);

            /* Check the error flag to see if errors occurred */
            if (sCANMessage.ui32Flags & MSG_OBJ_DATA_LOST)
            {
                  UARTprintf("\nCAN message loss detected\n");
            }
            else
            {
                /* Print a message to the console showing the message count and the
                 * contents of the message being sent */
                UARTprintf("Received msg 0x%03X: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                           sCANMessage.ui32MsgID, msgData[0], msgData[1], msgData[2],
                           msgData[3], msgData[4], msgData[5], msgData[6], msgData[7]);

                /* Print the count of message sent */
                UARTprintf(" total count = %u\n", msgCount);

                /* Toggle LED depending on msgData[0] */
                if (msgData[0])
                {
                    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
                }
                else
                {
                    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
                }
            }

            /* Clear rx flag */
            rxMsg = false;
        }
        else
        {
            if(errFlag)
            {
            }
        }
    }
}

void configureUART(void)
{
    /* Configure the UART and its pins.
     * This must be called before UARTprintf() */

    /* Enable the GPIO Peripheral used by the UART */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)))
    {
    }

    /* Enable UART2 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    /* Configure GPIO Pins for UART mode */
    MAP_GPIOPinConfigure(GPIO_PD4_U2RX);
    MAP_GPIOPinConfigure(GPIO_PD5_U2TX);
    MAP_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /* Initialize the UART for console I/O */
    UARTStdioConfig(2, 115200, sysClock);
}

void configureCAN(void)
{
    /* Configure the CAN and its pins PA0 and PA1 @ 500Kbps */

    /* Enable the clock to the GPIO Port A and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Enable CAN0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    /* Configure GPIO Pins for CAN mode */
    MAP_GPIOPinConfigure(GPIO_PA0_CAN0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_CAN0TX);
    MAP_GPIOPinTypeCAN(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Initialize the CAN controller */
    MAP_CANInit(CAN0_BASE);

    /* Set up the bit rate for the CAN bus.  CAN bus is set to 500 Kbps */
    MAP_CANBitRateSet(CAN0_BASE, sysClock, 500000);

    /* Enable interrupts on the CAN peripheral */
    MAP_CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    /* Enable the CAN interrupt */
    MAP_IntEnable(INT_CAN0);

    /* Enable the CAN for operation */
    MAP_CANEnable(CAN0_BASE);
}

void CAN0_IRQHandler(void)
{
    uint32_t canStatus;

    /* Read the CAN interrupt status to find the cause of the interrupt */
    canStatus = MAP_CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    /* If the cause is a controller status interrupt, then get the status */
    if(canStatus == CAN_INT_INTID_STATUS)
    {
        /* Read the controller status.  This will return a field of status
         * error bits that can indicate various errors */
        canStatus = MAP_CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        /* Set a flag to indicate some errors may have occurred */
        errFlag = true;
    }

    /* Check if the cause is message object 1, which what we are using for
     * receiving messages */
    else if(canStatus == 1)
    {
        /* Getting to this point means that the RX interrupt occurred on
         * message object 1, and the message RX is complete.  Clear the
         * message object interrupt */
        MAP_CANIntClear(CAN0_BASE, 1);

        /* Increment a counter to keep track of how many messages have been
         * sent. In a real application this could be used to set flags to
         * indicate when a message is sent */
        msgCount++;

        /* Set flag to indicate received message is pending */
        rxMsg = true;

        /* Since the message was sent, clear any error flags */
        errFlag = false;
    }
    else
    {
    }
}
