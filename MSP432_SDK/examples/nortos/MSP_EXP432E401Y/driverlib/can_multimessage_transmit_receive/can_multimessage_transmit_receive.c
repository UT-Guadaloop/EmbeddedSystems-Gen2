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
 * and transmit multiple messages on the CAN bus.
 *
 * This example periodically (10ms) sends CAN messages with ID's of 0x200 and
 * 0x201 each message has a length of 2 and contains the status of SW1
 * and SW2 (pressed or not pressed).
 *
 * Byte 1 of the message contains the status of SW1, for example:
 *      0x200   0xB1    0x00  --> SW1 not pressed
 *      0x200   0xB1    0x01  --> SW1 pressed
 *
 *      0x201   0xB2    0x00  --> SW2 not pressed
 *      0x201   0xB2    0x01  --> SW2 pressed
 *
 * Also this example expects to receive a CAN message with the IDs of 0x300 to
 * 0x303 and depending on the value of byte 1, it will turn ON/OFF their
 * corresponding LED, for example:
 *
 *      0x300   0xD1    0x00  --> LED1 OFF
 *      0x300   0xD1    0x01  --> LED1 ON
 *
 *      0x301   0xD2    0x00  --> LED2 OFF
 *      0x301   0xD2    0x01  --> LED2 ON
 *
 *      0x302   0xD3    0x00  --> LED3 OFF
 *      0x302   0xD3    0x01  --> LED3 ON
 *
 * At the same time, this example will toggle LED4 every 1 second and it uses
 * the UART to display if there was an error while transmitting or receiving a
 * CAN frame.
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
 *            |               PJ0|<-- SW1
 *            |               PJ1|<-- SW2
 *            |                  |
 *            |               PN1|--> LED1
 *            |               PN0|--> LED2
 *            |               PF4|--> LED3
 *            |               PF0|--> LED4
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

/* Demo CAN messages Buttons and LEDs */
uint8_t msg1TxData[2] = {0xB1, 0x00};
uint8_t msg2TxData[2] = {0xB2, 0x00};

uint8_t msgRxData[8] = {0x00};

/* Two CAN objects for Tx and One for Rx */
tCANMsgObject sCANTxMessage[2];
tCANMsgObject sCANRxMessage;

/* Scheduler variables */
uint8_t schedulerTimer;
uint8_t counter10ms;
uint8_t counter100ms;
uint8_t counter1s;

/* LP LEDs */
typedef enum
{
    LED1,
    LED2,
    LED3,
    LED4
} ledNum_t;

/* Button status */
bool isSW1Pressed = false;
bool isSW2Pressed = false;

/* Function prototypes */
void configureUART(void);
void configureCAN(void);
void configureBUTTONS(void);
void configureLEDS(void);
void turnLEDOn(ledNum_t ledNum);
void turnLEDOff(ledNum_t ledNum);
void toggleLED(ledNum_t ledNum);
void receiveCANMsgManager(void);
void configureSchedulerTimer(void);

int main(void)
{
    /* Run from the PLL at 120 MHz */
    sysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

    /* Initialize the UART */
    configureUART();

    /* Initialize board external interrupts for SW1 and SW2 */
    configureBUTTONS();

    /* Initialize board LEDs D1, D2, D3 and D4 */
    configureLEDS();

    /* Initialize the CAN */
    configureCAN();

    /* CAN TX */
    /* Initialize message objects to be able to send CAN message 1 */
    sCANTxMessage[0].ui32MsgID = 0x200;
    sCANTxMessage[0].ui32MsgIDMask = 0;                  /* No mask needed for TX  */
    sCANTxMessage[0].ui32Flags = MSG_OBJ_TX_INT_ENABLE;  /* Enable interrupt on TX */
    sCANTxMessage[0].ui32MsgLen = sizeof(msg1TxData);    /* Size of message        */
    sCANTxMessage[0].pui8MsgData = msg1TxData;           /* Ptr to message content */

    /* Initialize message objects to be able to send CAN message 2 */
    sCANTxMessage[1].ui32MsgID = 0x201;
    sCANTxMessage[1].ui32MsgIDMask = 0;                  /* No mask needed for TX  */
    sCANTxMessage[1].ui32Flags = MSG_OBJ_TX_INT_ENABLE;  /* Enable interrupt on TX */
    sCANTxMessage[1].ui32MsgLen = sizeof(msg2TxData);    /* Size of message        */
    sCANTxMessage[1].pui8MsgData = msg2TxData;           /* Ptr to message content */

    /* CAN RX */
    /* Initialize message object 3 to be able to receive CAN message ID 0x300
     * 0x301, 0x302 and 0x0303*/
    sCANRxMessage.ui32MsgID = 0;
    sCANRxMessage.ui32MsgIDMask = ~(0x303);              /* Look for specific ID's */
    sCANRxMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE      /* Enable interrupt on RX */
            | MSG_OBJ_USE_ID_FILTER;                     /* and Filter ID          */
    sCANRxMessage.ui32MsgLen = sizeof(msgRxData);        /* Size of message        */

    /* Load the message object into the CAN peripheral. Once loaded an
     * interrupt will occur only when a valid CAN ID is received. Use message
     * object 3 for receiving messages */
    MAP_CANMessageSet(CAN0_BASE, 3, &sCANRxMessage, MSG_OBJ_TYPE_RX);

    /* Initialize the timer for the scheduler */
    configureSchedulerTimer();

    /* Loop forever */
    while(1)
    {
        /*  10 ms */
        if (schedulerTimer >= 1)
        {
            schedulerTimer = 0;
            counter10ms++;

            /* Update button status in the CAN message */
            msg1TxData[1] = isSW1Pressed;
            msg2TxData[1] = isSW2Pressed;

            /* Send the CAN messages with the button status */
            MAP_CANMessageSet(CAN0_BASE, 1, &sCANTxMessage[0], MSG_OBJ_TYPE_TX);
            MAP_CANMessageSet(CAN0_BASE, 2, &sCANTxMessage[1], MSG_OBJ_TYPE_TX);
        }

        /* 20 ms */
        if (counter10ms >= 2)
        {
            counter10ms = 0;
            counter100ms++;

            /* Process received messages */
            receiveCANMsgManager();
        }

        /* 100 ms */
        if (counter100ms >= 5)
        {
            counter100ms = 0;
            counter1s++;
        }

        /* 1 sec */
        if (counter1s >= 10)
        {
            counter1s = 0;

            /* Toggle LED4  every 1 second */
            toggleLED(LED4);
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

void configureBUTTONS(void)
{
    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0 and PJ1 as input */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOJ->PUR = GPIO_PIN_0 | GPIO_PIN_1;

    /* Configure Interrupt Generation by Port Pin PJ0 and PJ1 for both edges */
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_BOTH_EDGES);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0 | GPIO_PIN_1);
    MAP_IntEnable(INT_GPIOJ);
}

void configureLEDS(void)
{
    /* Enable the GPIO port that is used for the on-board LED */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /* Check if the peripheral access is enabled */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    /* Check if the peripheral access is enabled */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    /* Enable the GPIO pins for the LEDs (PN0 and PN1). Set the direction as output,
     * and enable the GPIO pin for digital function */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1,
                     ~(GPIO_PIN_0 | GPIO_PIN_1));
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Enable the GPIO pins for the LEDs (PF0 and PF4). Set the direction as output,
     * and enable the GPIO pin for digital function */
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4,
                     ~(GPIO_PIN_0 | GPIO_PIN_4));
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
}

void turnLEDOn(ledNum_t ledNum)
{
    switch(ledNum)
    {
      case LED1:
          MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
          break;
      case LED2:
          MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
          break;
      case LED3:
          MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
          break;
      case LED4:
          MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
          break;
    }
}

void turnLEDOff(ledNum_t ledNum)
{
    switch(ledNum)
    {
      case LED1:
          MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));
          break;
      case LED2:
          MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
          break;
      case LED3:
          MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, ~(GPIO_PIN_4));
          break;
      case LED4:
          MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
          break;
    }
}

void toggleLED(ledNum_t ledNum)
{
    switch(ledNum)
    {
      case LED1:
          if(MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1) == GPIO_PIN_1)
          {
              MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~(GPIO_PIN_1));
          }
          else
          {
              MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
          }
          break;
      case LED2:
          if(MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0) == GPIO_PIN_0)
          {
              MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
          }
          else
          {
              MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
          }
          break;
      case LED3:
          if(MAP_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == GPIO_PIN_4)
          {
              MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, ~(GPIO_PIN_4));
          }
          else
          {
              MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
          }
          break;
      case LED4:
          if(MAP_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == GPIO_PIN_0)
          {
              MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, ~(GPIO_PIN_0));
          }
          else
          {
              MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
          }
          break;
    }
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

void receiveCANMsgManager(void)
{
    /* A new message is received */
    if (rxMsg)
    {
        /* Re-use the same message object that was used earlier to configure
         * the CAN */
        sCANRxMessage.pui8MsgData = (uint8_t *)&msgRxData;

        /* Read the message from the CAN */
        MAP_CANMessageGet(CAN0_BASE, 3, &sCANRxMessage, 0);

        /* Check the error flag to see if errors occurred */
        if (sCANRxMessage.ui32Flags & MSG_OBJ_DATA_LOST)
        {
            UARTprintf("\nCAN message loss detected\n");
        }
        else
        {
            switch(sCANRxMessage.ui32MsgID)
            {
            /* Check for a valid ID */
            case 0x300:
                /* Check for a valid data in the message */
                if (msgRxData[0] == 0xD1)
                {
                    /* Depending of the value of this byte, toggle LED */
                    if (msgRxData[1] == 0x00)
                    {
                        turnLEDOff(LED1);
                    }
                    else
                    {
                        turnLEDOn(LED1);
                    }
                }
                break;

                /* Check for a valid ID */
            case 0x301:
                /* Check for a valid data in the message */
                if (msgRxData[0] == 0xD2)
                {
                    /* Depending of the value of this byte, toggle LED */
                    if (msgRxData[1] == 0x00)
                    {
                        turnLEDOff(LED2);
                    }
                    else
                    {
                        turnLEDOn(LED2);
                    }
                }
                break;

                /* Check for a valid ID */
            case 0x302:
                /* Check for a valid data in the message */
                if (msgRxData[0] == 0xD3)
                {
                    /* Depending of the value of this byte, toggle LED */
                    if (msgRxData[1] == 0x00)
                    {
                        turnLEDOff(LED3);
                    }
                    else
                    {
                        turnLEDOn(LED3);
                    }
                }
                break;

            default:
                break;
            }
        }

        /* Clear rx flag */
        rxMsg = false;
    }
    else
    {
        if(errFlag)
        {
            UARTprintf("error while process the message\n");
            while(errFlag);
        }
    }
}

void GPIOJ_IRQHandler(void)
{
    uint32_t getSwitchState;

    /* Read the interrupt status register to find which switch button was
     * pressed and clear the interrupt status */
    getSwitchState = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    MAP_GPIOIntClear(GPIO_PORTJ_BASE, getSwitchState);

    /* If User Switch SW1 is pressed */
    if(getSwitchState & GPIO_PIN_0)
    {
        if(isSW1Pressed)
        {
            isSW1Pressed = false;
        }
        else
        {
            isSW1Pressed = true;
        }
    }

    /* If User Switch SW2 is pressed */
    if(getSwitchState & GPIO_PIN_1)
    {
        if(isSW2Pressed)
        {
            isSW2Pressed = false;
        }
        else
        {
            isSW2Pressed = true;
        }
    }
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
     * receiving messages */
    else if(canStatus == 3)
    {
        /* Getting to this point means that the RX interrupt occurred on
         * message object 3, and the message RX is complete.  Clear the
         * message object interrupt */
        MAP_CANIntClear(CAN0_BASE, 3);

        /* Set flag to indicate received message is pending */
        rxMsg = true;

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
