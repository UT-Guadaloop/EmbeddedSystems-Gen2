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
 * MSP432E4 Boot Ethernet - Application example for Ethernet Boot Loader
 *
 * Description: This example application demonstrates the operation of an
 * application image using the flash based ethernet boot loader. The
 * application uses the lwIP TCP/IP stack and puts a callback function on UDP
 * Port 9 to receive update from a remote PC for boot request. It then jumps
 * to the boot loader.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|             PN1  |--> LED
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>

/* Project specific Includes */
#include "ustdlib.h"
#include "uartstdio.h"
#include "swupdate.h"
#include "lwiplib.h"

/* Defines for setting up the SysTick module. */
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

/* A global flag used to indicate if a remote firmware update request has been
 * received */
static volatile bool g_bFirmwareUpdate = false;

/* A global flag used to indicate if an IP address has been acquired using DHCP */
static volatile bool g_bIPAddressAcquired = false;

/* The current IP address. */
uint32_t setCurrentIPAddress;

/* Configure the UART-0 for display with 8-N-1 format and 115200 bps */
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

/* Display an lwIP type IP Address. */
void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    /* Convert the IP Address into a string. */
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    /* Display the string. */
    UARTprintf(pcBuf);
}

/* Required by lwIP library to support any host-related timer functions. */
void
lwIPHostTimerHandler(void)
{
    uint32_t getNewIPAddress;

    /* Get the current IP address. */
    getNewIPAddress = lwIPLocalIPAddrGet();

    /* See if the IP address has changed. */
    if(getNewIPAddress != setCurrentIPAddress)
    {
        /* See if there is an IP address assigned. */
        if(getNewIPAddress == 0xffffffff)
        {
            /* Indicate that there is no link. */
            UARTprintf("Waiting for link.\n");
        }
        else if(getNewIPAddress == 0)
        {
            /* There is no IP address, so indicate that the DHCP process is
             * running. */
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            /* Display the new IP address. and set the flag that the IP
             * address has been acquired. */
            UARTprintf("IP Address: ");
            DisplayIPAddress(getNewIPAddress);
            g_bIPAddressAcquired = true;
        }

        /* Save the new IP address. */
        setCurrentIPAddress = getNewIPAddress;
    }

    /* If there is not an IP address. */
    if((getNewIPAddress == 0) || (getNewIPAddress == 0xffffffff))
    {
        /* Do nothing and keep waiting. */
    }
}

/* The interrupt handler for the SysTick interrupt. */
void
SysTick_Handler(void)
{
    /* Call the lwIP timer handler. */
    lwIPTimer(SYSTICKMS);
}

/* This function is called by the software update module whenever a remote
 * host requests to update the firmware on this board.  We set a flag that
 * will cause the main loop to exit and transfer control to the bootloader.
 *
 * IMPORTANT:
 * Note that this callback is made in interrupt context and, since it is not
 * permitted to transfer control to the boot loader from within an interrupt,
 * we can't just call SoftwareUpdateBegin() here. */
void SoftwareUpdateRequestCallback(void)
{
    g_bFirmwareUpdate = true;
}

/* Perform the initialization steps required to start up the Ethernet controller
 * and lwIP stack. */
void
SetupForEthernet(void)
{
    /* Start the remote software update module. */
    SoftwareUpdateInit(SoftwareUpdateRequestCallback);
}

/* Main application code */
int
main(void)
{
    uint32_t getSystemClock;
    uint32_t getUser0, getUser1;
    uint8_t  pui8MACArray[8];

    /* Run from the PLL at 120 MHz. */
    getSystemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    /* Configure the serial console for display */
    ConfigureUART(getSystemClock);

    /* Clear the terminal and print banner. */
    UARTprintf("\033[2J\033[H");
    UARTprintf("Ethernet Boot Application: Magic Packet Detect\n");
    UARTprintf("to invoke the flash based boot loader...\n\n");

    /* Configure SysTick for a periodic interrupt. */
    MAP_SysTickPeriodSet(getSystemClock / SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    /* Configure the hardware MAC address for Ethernet Controller filtering of
     * incoming packets.  The MAC address will be stored in the non-volatile
     * USER0 and USER1 registers. */
    MAP_FlashUserGet(&getUser0, &getUser1);
    if((getUser0 == 0xffffffff) || (getUser1 == 0xffffffff))
    {
        /* We should never get here.  This is an error if the MAC address has
         * not been programmed into the device.  Exit the program. Let the
         * user know there is no MAC address */
        UARTprintf("No MAC programmed!\n");
        while(1)
        {
        }
    }

    /* Tell the user what we are doing just now. */
    UARTprintf("Waiting for IP.\n");

    /* Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
     * address needed to program the hardware registers, then program the MAC
     * address into the Ethernet Controller registers. */
    pui8MACArray[0] = ((getUser0 >>  0) & 0xff);
    pui8MACArray[1] = ((getUser0 >>  8) & 0xff);
    pui8MACArray[2] = ((getUser0 >> 16) & 0xff);
    pui8MACArray[3] = ((getUser1 >>  0) & 0xff);
    pui8MACArray[4] = ((getUser1 >>  8) & 0xff);
    pui8MACArray[5] = ((getUser1 >> 16) & 0xff);

    /* Initialize the lwIP library, using DHCP. */
    lwIPInit(getSystemClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    /* Wait for the IP Address to be acquired */
    while(!g_bIPAddressAcquired)
    {
    }

    /* Print the status message saying that we are ready for another update */
    UARTprintf("\n\nDevice ready for detecting Magic Packet on UDP Port 9\n");

    /* Setup the call back function for magic packet detect on UDP Port 9 */
    SetupForEthernet();

    /* Configure Port N pin 1 as output. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)));
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    /* If the firmware request is not issued by the Host then blink the LED D1
     * On switch firmwar request detect exit the blinking program and jump to
     * the flash boot loader. */
    while(!g_bFirmwareUpdate)
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);
        MAP_SysCtlDelay(getSystemClock / 6);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
        MAP_SysCtlDelay(getSystemClock / 6);
    }

    /* Before passing control make sure that the LED is turned OFF. */
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);

    /* Pass control to whichever flavour of boot loader the board is configured
     * with. */
    SoftwareUpdateBegin(getSystemClock);

    /* The previous function never returns but we need to stick in a return
     * code here to keep the compiler from generating a warning. */
    return(0);
}
