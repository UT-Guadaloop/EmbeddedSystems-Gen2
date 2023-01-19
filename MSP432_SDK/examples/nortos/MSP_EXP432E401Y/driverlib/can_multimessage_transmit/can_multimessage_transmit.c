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
 * MSP432E4 Example Project for CAN Multi Message Transmit
 *
 * Description: This example shows the basic setup of CAN in order to transmit
 * multiple messages on the CAN bus at different rate.
 *
 * This example periodically sends CAN messages with ID's of 0x100(10ms),
 * 0x101(20ms), 0x102(100ms) and 0x103(1 second) and each message has a length
 * of 8. It uses the UART to display if there was an error while transmitting
 * the CAN frame.
 *
 * Note: Requires CAN transceiver and CAN sniffer tool
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
bool errFlag = false;

/* Scheduler variables */
uint8_t schedulerTimer;
uint8_t counter10ms;
uint8_t counter100ms;
uint8_t counter1s;

/* Function prototypes */
void configureUART(void);
void configureCAN(void);
void configureSchedulerTimer(void);

int main(void)
{
    tCANMsgObject sCANMessage[4];
    uint8_t msgData[8] = {0x01, 0x02, 0x03, 0x04,
                          0x05, 0x06, 0x07, 0x08};

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

    /* Enable the GPIO pin for the LED (PN0).  Set the direction as output, and
     * enable the GPIO pin for digital function */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    /* Initialize the CAN */
    configureCAN();

    /* Initialize message objects to be able to send CAN message 1 */
    sCANMessage[0].ui32MsgID = 0x100;
    sCANMessage[0].ui32MsgIDMask = 0;                  /* No mask needed for TX  */
    sCANMessage[0].ui32Flags = MSG_OBJ_TX_INT_ENABLE;  /* Enable interrupt on TX */
    sCANMessage[0].ui32MsgLen = sizeof(msgData);       /* Size of message is 8   */
    sCANMessage[0].pui8MsgData = msgData;              /* Ptr to message content */

    /* Initialize message objects to be able to send CAN message 2 */
    sCANMessage[1].ui32MsgID = 0x101;
    sCANMessage[1].ui32MsgIDMask = 0;                  /* No mask needed for TX  */
    sCANMessage[1].ui32Flags = MSG_OBJ_TX_INT_ENABLE;  /* Enable interrupt on TX */
    sCANMessage[1].ui32MsgLen = sizeof(msgData);       /* Size of message is 8   */
    sCANMessage[1].pui8MsgData = msgData;              /* Ptr to message content */

    /* Initialize message objects to be able to send CAN message 3 */
    sCANMessage[2].ui32MsgID = 0x102;
    sCANMessage[2].ui32MsgIDMask = 0;                  /* No mask needed for TX  */
    sCANMessage[2].ui32Flags = MSG_OBJ_TX_INT_ENABLE;  /* Enable interrupt on TX */
    sCANMessage[2].ui32MsgLen = sizeof(msgData);       /* Size of message is 8   */
    sCANMessage[2].pui8MsgData = msgData;              /* Ptr to message content */

    /* Initialize message objects to be able to send CAN message 4 */
    sCANMessage[3].ui32MsgID = 0x103;
    sCANMessage[3].ui32MsgIDMask = 0;                  /* No mask needed for TX  */
    sCANMessage[3].ui32Flags = MSG_OBJ_TX_INT_ENABLE;  /* Enable interrupt on TX */
    sCANMessage[3].ui32MsgLen = sizeof(msgData);       /* Size of message is 8   */
    sCANMessage[3].pui8MsgData = msgData;              /* Ptr to message content */

    /* Initialize the timer for the scheduler */
    configureSchedulerTimer();

    /* Scheduler */
    while(1)
    {
        /*  10 ms */
        if (schedulerTimer >= 1)
        {
            schedulerTimer = 0;
            counter10ms++;

            /* Send the CAN message using object number 1 */
            MAP_CANMessageSet(CAN0_BASE, 1, &sCANMessage[0], MSG_OBJ_TYPE_TX);
        }

        /* 20 ms */
        if (counter10ms >= 2)
        {
            counter10ms = 0;
            counter100ms++;

            /* Send the CAN message using object number 2 */
            MAP_CANMessageSet(CAN0_BASE, 2, &sCANMessage[1], MSG_OBJ_TYPE_TX);
        }

        /* 100 ms */
        if (counter100ms >= 5)
        {
            counter100ms = 0;
            counter1s++;

            /* Send the CAN message using object number 3 */
            MAP_CANMessageSet(CAN0_BASE, 3, &sCANMessage[2], MSG_OBJ_TYPE_TX);
        }

        /* 1 sec */
        if (counter1s >= 10)
        {
            counter1s = 0;

            /* Send the CAN message using object number 4 */
            MAP_CANMessageSet(CAN0_BASE, 4, &sCANMessage[3], MSG_OBJ_TYPE_TX);

            /* Toggle LED  every 1 second */
            if(MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0) == GPIO_PIN_0)
            {
                MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
            }
            else
            {
                MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
            }
        }

        /* Check the error flag to see if errors occurred */
        if(errFlag)
        {
            UARTprintf("error - cable connected?\n");
            while(errFlag);
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

    /* Enable auto-retry on CAN transmit */
    MAP_CANRetrySet(CAN0_BASE, true);

    /* Enable the CAN interrupt */
    MAP_IntEnable(INT_CAN0);

    /* Enable the CAN for operation */
    MAP_CANEnable(CAN0_BASE);
}

void configureSchedulerTimer(void)
{
    uint32_t timerPeriod;

    /* Enable Timer peripheral */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    /* Configure TIMER0 / TIMER_A as periodic timer */
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    /* 100Hz - 10ms */
    timerPeriod = (sysClock / 100);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, timerPeriod - 1);

    /* Enable interrupt for the timer timeout */
    MAP_IntEnable(INT_TIMER0A);
    MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);
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
     * sending messages */
    else if(canStatus == 1)
    {
        /* Getting to this point means that the TX interrupt occurred on
         * message object 1, and the message TX is complete.  Clear the
         * message object interrupt */
        MAP_CANIntClear(CAN0_BASE, 1);

        /* Since the message was sent, clear any error flags */
        errFlag = false;
    }

    /* Check if the cause is message object 2, which what we are using for
     * sending messages */
    else if(canStatus == 2)
    {
        /* Getting to this point means that the TX interrupt occurred on
         * message object 2, and the message TX is complete.  Clear the
         * message object interrupt */
        MAP_CANIntClear(CAN0_BASE, 2);

        /* Since the message was sent, clear any error flags */
        errFlag = false;
    }

    /* Check if the cause is message object 3, which what we are using for
     * sending messages */
    else if(canStatus == 3)
    {
        /* Getting to this point means that the TX interrupt occurred on
         * message object 3, and the message TX is complete.  Clear the
         * message object interrupt */
        MAP_CANIntClear(CAN0_BASE, 3);

        /* Since the message was sent, clear any error flags */
        errFlag = false;
    }

    /* Check if the cause is message object 4, which what we are using for
     * sending messages */
    else if(canStatus == 4)
    {
        /* Getting to this point means that the TX interrupt occurred on
         * message object 4, and the message TX is complete.  Clear the
         * message object interrupt */
        MAP_CANIntClear(CAN0_BASE, 4);

        /* Since the message was sent, clear any error flags */
        errFlag = false;
    }
    else
    {
    }
}

void TIMER0A_IRQHandler(void)
{
    /* Clear the timer interrupt */
    MAP_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* Increment scheduler */
    schedulerTimer++;
}
