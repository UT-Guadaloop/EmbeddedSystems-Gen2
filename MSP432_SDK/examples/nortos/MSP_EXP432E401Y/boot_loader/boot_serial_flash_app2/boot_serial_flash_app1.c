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
 * MSP432E4 Boot - Application example for Serial boot loaders of UART, SSI,
 * I2C & USB.
 *
 * Description: This very simple code example shows how an application code
 * can be downloaded using the boot loader. On being flashed to Sector-1 of the
 * flash the application code initalizes the boot interfaces and then toggles
 * an LED. When the User Switch SW1 is pressed, it stops the LED blinking and
 * jumps back to the boot loader for a new firmware image.
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|             PA0  |<---- UART0 RX
 *          | |             PA1  |----> UART0 TX
 *          --|RST               |
 *            |             PB2  |<---> I2C0 SCL
 *   USR_SW1--|PJ0          PB3  |<---> I2C0 SDA
 *            |                  |
 *       LED--|PN1          PA2  |<---- SSI0 CLK
 *            |             PA3  |<---- SSI0 FSS
 *            |             PA4  |----> SSI0 MISO
 *            |             PA5  |<---- SSI0 MOSI
 *            |                  |
 *            |             PL6  |<---> USB DP
 *            |             PL7  |<---> USB DM
 *            |                  |
 *
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

volatile uint32_t g_ui32SysClockFreq;

//*****************************************************************************
//
// Function call to jump to the boot loader.
//
//*****************************************************************************
void
JumpToBootLoader(void)
{
    /* We must make sure we turn off SysTick and its interrupt before entering
     * the boot loader! */
    MAP_SysTickIntDisable();
    MAP_SysTickDisable();

    /* Disable all processor interrupts.  Instead of disabling them one at a
     * time, a direct write to NVIC is done to disable all peripheral
     * interrupts. */
    NVIC->ICER[0] = 0xffffffff;
    NVIC->ICER[1] = 0xffffffff;
    NVIC->ICER[2] = 0xffffffff;
    NVIC->ICER[3] = 0xffffffff;

    /* Return control to the boot loader.  This is a call to the SVC handler
     * in the boot loader. */
    (*((void (*)(void))(*(uint32_t *)0x2c)))();
}

//*****************************************************************************
//
// Initialize UART0 and set the appropriate communication parameters.
//
//*****************************************************************************
void
SetupForUART(void)
{
    /* We need to make sure that UART0 and its associated GPIO port are
     * enabled before we pass control to the boot loader.  The serial boot
     * loader does not enable or configure these peripherals for us if we
     * enter it via its SVC vector. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Set GPIO PA0 and PA1 as UART. */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200, n, 8, 1 */
    MAP_UARTConfigSetExpClk(UART0_BASE, g_ui32SysClockFreq, 115200,
                            (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_WLEN_8));

    /* Enable the UART operation. */
    MAP_UARTEnable(UART0_BASE);
}

//*****************************************************************************
//
// Initialize I2C0 and set the appropriate communication parameters.
//
//*****************************************************************************
void
SetupForI2C(void)
{
    /* We need to make sure that UART0 and its associated GPIO port are
     * enabled before we pass control to the boot loader.  The serial boot
     * loader does not enable or configure these peripherals for us if we
     * enter it via its SVC vector. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    /* Set GPIO PB2 and PB3 as I2C. Setting the internal weak pull up and
     * clearing the internal weak pull down in case there is no board level
     * pull up is not provided. This is not a preferred method but will
     * provide some functionality. It is strongly advised to have board pull
     * up of 1.0K to 3.3K for proper operation of the I2C bus. */
    MAP_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    MAP_GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    MAP_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    MAP_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    GPIOB->PUR |= (GPIO_PIN_3 | GPIO_PIN_2);
    GPIOB->PDR &= !(GPIO_PIN_3 | GPIO_PIN_2);


    /* Configure the I2C for slave mode with Slave address of 0x42 */
    MAP_I2CSlaveInit(I2C0_BASE, 0x42);
}

//*****************************************************************************
//
// Initialize SSI0 and set the appropriate communication parameters.
//
//*****************************************************************************
void
SetupForSSI(void)
{
    /* We need to make sure that UART0 and its associated GPIO port are
     * enabled before we pass control to the boot loader.  The serial boot
     * loader does not enable or configure these peripherals for us if we
     * enter it via its SVC vector. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    /* Set GPIO PA2, PA3, PA4 and PA5 as SSI.  */
    MAP_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    MAP_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    MAP_GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);
    MAP_GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);
    MAP_GPIOPinTypeSSI(GPIO_PORTA_BASE, (GPIO_PIN_2 | GPIO_PIN_3 |
                                        GPIO_PIN_4 | GPIO_PIN_5));

    /* Configure the SSI Slave mode with 1 Mbps data rate*/
    MAP_SSIConfigSetExpClk(SSI0_BASE, g_ui32SysClockFreq,
                           SSI_FRF_MOTO_MODE_0, SSI_MODE_SLAVE,
                           1000000, 8);

    /* Enable the UART operation. */
    MAP_SSIEnable(SSI0_BASE);
}

//*****************************************************************************
//
// Initialize USB0 and set the appropriate communication parameters.
//
//*****************************************************************************
void
SetupForUSB(void)
{
    /* No specific initialization required for USB as the boot loader takes
     * care of initializing the USB peripheral and pins */
}

//*****************************************************************************
//
// Main application code.
//
//*****************************************************************************
int main(void)
{
    /* Set the system clock to run at 120MHz from the PLL */
    g_ui32SysClockFreq = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    /* Initialize the peripherals that each of the boot loader flavor
     * supports.  Since this example is intended for use with any of the
     * boot loaders and we don't know which is actually in use, we cover all
     * bases and initialize for serial, Ethernet and USB use here. */
    SetupForUART();
    SetupForI2C();
    SetupForSSI();
    SetupForUSB();

    /* Enable Port J Pin 0 for exit to UART Boot loader when Pressed Low.
     * On the MSP_EXP432E401Y the weak pull up is enabled to detect button
     * press. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)));
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR = GPIO_PIN_0;

    /* Configure Port N pin 1 as output. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    /* If the Switch SW1 is not pressed then blink the LED D1 at 1 Hz rate
     * On switch SW press detection exit the blinking program and jump to
     * the flash boot loader. */
    while((GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) & GPIO_PIN_0) != 0x0)
    {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);
        SysCtlDelay(g_ui32SysClockFreq / 6);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(g_ui32SysClockFreq / 6);
    }

    /* Before passing control make sure that the LED is turned OFF. */
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);

    /* Pass control to whichever flavor of boot loader the board is configured
     * with. */
    JumpToBootLoader();

    /* The previous function never returns but we need to stick in a return
     * code here to keep the compiler from generating a warning. */
    return(0);

}
