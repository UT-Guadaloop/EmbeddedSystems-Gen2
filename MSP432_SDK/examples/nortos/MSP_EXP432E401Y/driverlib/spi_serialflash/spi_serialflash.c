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
 * MSP432E4 Example Project for QSSI with Serial Flash.
 *
 * Description: Example demonstrating how to configure the QSSI module for
 * accessing a Serial Flash in Adv-Bi-Quad Mode. It uses the UART to display
 * status messages.
 *
 * This example will send out 256 bytes in Advanced, Bi and Quad Mode and then
 * read the data in Advanced, Bi and Quad Mode. Once the data check is completed
 * it shall Erase the Serial Flash to return the device to it's original state
 *
 * Note: Requires external Quad Serial Flash (MX66L51235F - 512M-BIT). Similar
 * to the SDRAM-NVM Daughtercard (http://www.ti.com/tool/TIDM-TM4C129SDRAMNVM)
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PB5|--> SSI1CLK
 *            |               PB4|--> SSI1FSS
 *            |               PE4|<-> SSI1XDAT0
 *            |               PE5|<-> SSI1XDAT1
 *            |               PD4|<-> SSI1XDAT2
 *            |               PD5|<-> SSI1XDAT3
 *            |                  |
 *
 * Author: David Lara
*******************************************************************************/
/* DriverLib Includes */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "uartstdio.h"

/* The number of Bytes to be sent or received */
#define NUM_SSI_DATA                256

/* The QSSI Flash Memory Access Instructions */
#define INS_WRITE_ENABLE            0x06
#define INS_WRITE_STATUS_REGISTER   0x01
#define INS_READ_STATUS_REGISTER1   0x05
#define INS_SECTOR_ERASE_4KB        0x20
#define INS_ADV_PAGE_PROGRAM        0x02
#define INS_QUAD_PAGE_PROGRAM       0x38
#define INS_ADV_READ_DATA           0x03
#define INS_BI_READ_DATA            0x3B
#define INS_QUAD_READ_DATA          0x6B

/* Dummy Byte when Padding or during Receive operations */
#define DUMMY_BYTE                  0x00

/* The QSSI Flash memory Status Register Bits for Polling Operations */
#define FLASH_WIP_BIT               0x01
#define FLASH_WEL_BIT               0x02
#define FLASH_QE_BIT                0x40

/* The QSSI Flash memory Configuration Register Bits Default State */
#define FLASH_CFG_REG               0x07

/* System clock rate in Hz */
uint32_t getSystemClock;

/* Transmit and Receive Buffers */
uint32_t setTxData[NUM_SSI_DATA];
uint32_t getRxData[NUM_SSI_DATA];
bool     errorFlag;

/* Function prototypes */
void configureUART(void);
void configureSerialFlashSSI(void);
void SSILibDeviceBusyCheck(uint32_t ui32Base);
void SSILibSendEraseCommand (uint32_t ui32Base, uint32_t ui32Address, uint8_t  ui8FlashCommand);
bool SSILibSendPageProgram(uint32_t ui32Base, uint32_t ui32Address, uint8_t ui8FlashCommand);
bool SSILibSendReadDataAdvBiQuad(uint32_t ui32Base, uint32_t ui32Address, uint8_t ui8FlashCommand);
uint8_t  SSILibSendReadStatusRegister(uint32_t ui32Base, uint8_t ui8FlashCommand);
uint32_t runSerialFlashSSIExample(void);
uint32_t SSILibSendReadIDAdvMode(uint32_t ui32Base);

int main(void)
{
    /* Run from the PLL at 120 MHz */
    getSystemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480),
                                            120000000);

    /* Initialize the UART */
    configureUART();

    /* Initialize SSI for the serial flash */
    configureSerialFlashSSI();

    /* Run the serial flash example  */
    runSerialFlashSSIExample();

    /* Halt the Program in a while loop */
    while(1);
}

void configureUART(void)
{
    /* Configure the UART and its pins. This must be called before
     * UARTprintf() */

    /* Enable the GPIO Peripheral used by the UART */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Enable UART0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure GPIO Pins for UART mode */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Initialize the UART for console I/O */
    UARTStdioConfig(0, 115200, getSystemClock);
}

void configureSerialFlashSSI(void)
{
    uint32_t dummyRead[1];

    /* Display the setup on the console */
    UARTprintf("\033[2J\033[H");
    UARTprintf("\rSSI1 To MSP432E4 Serial to Flash Example\n\n");

    /* The SSI1 peripheral must be disabled, reset and re enabled for use
     * Wait till the Peripheral ready is not asserted */
    MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_SSI1);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_SSI1);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI1)))
    {
    }


    /* For this example SSI1 is used with the following GPIO Pin Mapping
     * SSI1CLK   : PB5
     * SSI1FSS   : PB4
     * SSI1XDAT0 : PE4
     * SSI1XDAT1 : PE5
     * SSI1XDAT2 : PD4
     * SSI1XDAT3 : PD5 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)))
    {
    }

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)))
    {
    }

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }

    /* Configure the pin mux for SSI1 functions on port PB4, PB5, PE4,
     * PE5, PD4, PD5 */
    MAP_GPIOPinConfigure(GPIO_PB5_SSI1CLK);
    MAP_GPIOPinConfigure(GPIO_PB4_SSI1FSS);
    MAP_GPIOPinConfigure(GPIO_PE4_SSI1XDAT0);
    MAP_GPIOPinConfigure(GPIO_PE5_SSI1XDAT1);
    MAP_GPIOPinConfigure(GPIO_PD4_SSI1XDAT2);
    MAP_GPIOPinConfigure(GPIO_PD5_SSI1XDAT3);

    /* Configure the GPIO settings for the SSI pins.  This function also gives
     * control of these pins to the SSI hardware.  Consult the data sheet to
     * see which functions are allocated per pin */
    MAP_GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_5 | GPIO_PIN_4);
    MAP_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);
    MAP_GPIOPinTypeSSI(GPIO_PORTE_BASE, GPIO_PIN_5 | GPIO_PIN_4);

    /* No driverlib API to only set the GPIO high drive setting, so we use
     * bare metal */
    GPIOB->DR8R |= (GPIO_PIN_5 | GPIO_PIN_4);
    GPIOD->DR8R |= (GPIO_PIN_5 | GPIO_PIN_4);
    GPIOE->DR8R |= (GPIO_PIN_5 | GPIO_PIN_4);

    /* Configure and enable the SSI port for SPI master mode.  Use SSI1,
     * system clock supply, idle clock level low and active low clock in
     * freescale SPI mode, master mode, 60MHz SSI frequency, and 8-bit data.
     * For SPI mode, you can set the polarity of the SSI clock when the SSI
     * unit is idle.  You can also configure what clock edge you want to
     * capture data on.  Please reference the datasheet for more information on
     * the different SPI modes */
    MAP_SSIConfigSetExpClk(SSI1_BASE, getSystemClock, SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER, (getSystemClock / 2), 8);

    /* Enable the SSI1 module */
    MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
    MAP_SSIAdvFrameHoldEnable(SSI1_BASE);

    /* No driverlib API to enable the SSI High Speed Clock, so we use
     * bare metal */
    SSI1->CR1 |= SSI_CR1_HSCLKEN;
    MAP_SSIEnable(SSI1_BASE);

    /* Read any residual data from the SSI port.  This makes sure the receive
     * FIFOs are empty, so we don't read any unwanted junk.  This is done here
     * because the SPI SSI mode is full-duplex, which allows you to send and
     * receive at the same time.  The SSIDataGetNonBlocking function returns
     * "true" when data was returned, and "false" when no data was returned.
     * The "non-blocking" function checks if there is any data in the receive
     * FIFO and does not "hang" if there isn't */
    while(MAP_SSIDataGetNonBlocking(SSI1_BASE, &dummyRead[0]))
    {
    }
}

uint32_t runSerialFlashSSIExample(void)
{
    uint32_t ui32DeviceID;
    uint32_t ui32Index;

    /* Initialize the Transmit Buffer */
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        setTxData[ui32Index] = rand()%256;
        getRxData[ui32Index] = 0x0;
    }

    /* 1. Read the DEVICE ID */
    ui32DeviceID = SSILibSendReadIDAdvMode(SSI1_BASE);

    if(ui32DeviceID != 0xC219)
    {
        UARTprintf("ERROR: No External Flash... Read Back %x\n",ui32DeviceID);
        return (0);
    }

    UARTprintf("QSSI : External Flash Detected with Device ID 0x%x\n",ui32DeviceID);

    /* 2. Erase the Sector before Program Operation... */
    UARTprintf("ADV  : Starting Erase Operations...\n");

    SSILibSendEraseCommand(SSI1_BASE,0x0,INS_SECTOR_ERASE_4KB);

    UARTprintf("ADV  : Erase Completed...\n");

    /* Ensure Previous Operation is complete */
    SSILibDeviceBusyCheck(SSI1_BASE);

    /* 3. Read NUM_SSI_DATA words from the External Flash */
    UARTprintf("ADV  : Starting Read Operations for Erase ...\n");
    if(!(SSILibSendReadDataAdvBiQuad(SSI1_BASE,0x0,INS_ADV_READ_DATA)))
    {
        UARTprintf("ERROR: Wrong Command for Read Programming...\n");
        return (0);
    }

    for(ui32Index = 0; ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPut(SSI1_BASE,DUMMY_BYTE);
        MAP_SSIDataGet(SSI1_BASE,&getRxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,DUMMY_BYTE);
    MAP_SSIDataGet(SSI1_BASE,&getRxData[NUM_SSI_DATA-1]);
    UARTprintf("ADV  : Read Completed for Erase Serial Flash...\n");

    /* Check Data and Display if Mismatch for Erase Operation and
     * exit the main program */
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        if(getRxData[ui32Index] != 0xFF)
        {
            UARTprintf("ERROR: Erase Failure @ %d with value as %x\n",ui32Index, getRxData[ui32Index]);
            errorFlag = true;
        }
    }

    if(errorFlag)
    {
        return (0);
    }

    /* 4. Write NUM_SSI_DATA words to the External Flash in Advanced Mode */
    UARTprintf("=====================================\n");
    UARTprintf("ADV  : Starting Program Operations...\n");

    if(!(SSILibSendPageProgram(SSI1_BASE,0x0,INS_ADV_PAGE_PROGRAM)))
    {
        UARTprintf("ERROR: Wrong Command for Page Programming...\n");
        return (0);
    }

    for(ui32Index = 0; ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPut(SSI1_BASE,setTxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,setTxData[NUM_SSI_DATA-1]);

    /* Ensure Previous Operation is complete */
    SSILibDeviceBusyCheck(SSI1_BASE);

    /* 4a. Read data in Advanced Mode */
    SSILibSendReadDataAdvBiQuad(SSI1_BASE,0x0,INS_ADV_READ_DATA);

    for(ui32Index = 0;ui32Index<NUM_SSI_DATA-1;ui32Index++)
    {
        MAP_SSIDataPutNonBlocking(SSI1_BASE,DUMMY_BYTE);
        MAP_SSIDataGet(SSI1_BASE,&getRxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,DUMMY_BYTE);
    MAP_SSIDataGet(SSI1_BASE,&getRxData[NUM_SSI_DATA-1]);

    /* Check Data and Display if Mismatch */
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        if(setTxData[ui32Index] != getRxData[ui32Index])
        {
            UARTprintf("ERROR: Program: %x Read: %x\n",setTxData[ui32Index],getRxData[ui32Index]);
            errorFlag = true;
        }
    }

    if(errorFlag)
    {
        return (0);
    }

    UARTprintf("ADV  : Read Completed...\n");

    /* 5. Write NUM_SSI_DATA words to the External Flash in QUAD Mode */
    UARTprintf("=====================================\n");
    UARTprintf("QUAD : Starting Program Operations...\n");

    if(!(SSILibSendPageProgram(SSI1_BASE,0x100,INS_QUAD_PAGE_PROGRAM)))
    {
        UARTprintf("ERROR: Wrong Command for Page Programming...\n");
        return (0);
    }

    for(ui32Index = 0; ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPut(SSI1_BASE,setTxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,setTxData[NUM_SSI_DATA-1]);

    /* Ensure Previous Operation is complete */
    SSILibDeviceBusyCheck(SSI1_BASE);

    /* 5a. Read data in Bi Mode */
    SSILibSendReadDataAdvBiQuad(SSI1_BASE,0x100,INS_BI_READ_DATA);

    for(ui32Index = 0; ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPutNonBlocking(SSI1_BASE,DUMMY_BYTE);
        MAP_SSIDataGet(SSI1_BASE,&getRxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,DUMMY_BYTE);
    MAP_SSIDataGet(SSI1_BASE,&getRxData[NUM_SSI_DATA-1]);

    /* Check Data and Display if Mismatch */
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        if(setTxData[ui32Index] != getRxData[ui32Index])
        {
            UARTprintf("ERROR: Program: %x Read: %x\n",setTxData[ui32Index],getRxData[ui32Index]);
            errorFlag = true;
        }
    }

    if(errorFlag)
    {
        return (0);
    }

    UARTprintf("BI   : Read Completed...\n");

    /* 6. Write NUM_SSI_DATA words to the External Flash in Advanced Mode */
    UARTprintf("=====================================\n");
    UARTprintf("ADV  : Starting Program Operations...\n");

    if(!(SSILibSendPageProgram(SSI1_BASE,0x200,INS_ADV_PAGE_PROGRAM)))
    {
        UARTprintf("ERROR: Wrong Command for Page Programming...\n");
        return (0);
    }

    for(ui32Index = 0;ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPut(SSI1_BASE,setTxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,setTxData[NUM_SSI_DATA-1]);

    /* Ensure Previous Operation is complete */
    SSILibDeviceBusyCheck(SSI1_BASE);

    /* 6a. Read data in Quad Mode */
    SSILibSendReadDataAdvBiQuad(SSI1_BASE,0x200,INS_QUAD_READ_DATA);

    for(ui32Index = 0; ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPutNonBlocking(SSI1_BASE,DUMMY_BYTE);
        MAP_SSIDataGet(SSI1_BASE,&getRxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,DUMMY_BYTE);
    MAP_SSIDataGet(SSI1_BASE,&getRxData[NUM_SSI_DATA-1]);


    /* Check Data and Display if Mismatch */
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        if(setTxData[ui32Index] != getRxData[ui32Index])
        {
            UARTprintf("ERROR: Program: %x Read: %x\n",setTxData[ui32Index],getRxData[ui32Index]);
            errorFlag = true;
        }
    }

    if(errorFlag)
    {
        return (0);
    }

    UARTprintf("QUAD : Read Completed...\n");

    /* 7. Erase the external flash */
    UARTprintf("=====================================\n");
    UARTprintf("ADV  : Starting Erase Operations...\n");

    SSILibSendEraseCommand(SSI1_BASE,0x0,INS_SECTOR_ERASE_4KB);

    UARTprintf("ADV  : Erase Completed...\n");

    /* Read NUM_SSI_DATA words from the External Flash */
    UARTprintf("ADV  : Starting Read Operations for Erase ...\n");
    SSILibSendReadDataAdvBiQuad(SSI1_BASE,0x0,INS_ADV_READ_DATA);

    for(ui32Index = 0; ui32Index < (NUM_SSI_DATA-1); ui32Index++)
    {
        MAP_SSIDataPut(SSI1_BASE,DUMMY_BYTE);
        MAP_SSIDataGet(SSI1_BASE,&getRxData[ui32Index]);
    }

    MAP_SSIAdvDataPutFrameEnd(SSI1_BASE,DUMMY_BYTE);
    MAP_SSIDataGet(SSI1_BASE,&getRxData[NUM_SSI_DATA-1]);
    UARTprintf("ADV  : Read Completed for Erase Serial Flash...\n");

    /* Check Data and Display if Mismatch for Erase Operation */
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        if(getRxData[ui32Index] != 0xFF)
        {
            UARTprintf("ERROR: Erase Failure @ %d with value as %x\n",ui32Index, getRxData[ui32Index]);
            errorFlag = true;
        }
    }

    if(errorFlag)
    {
        return (0);
    }
    else
    {
        /* If code execution reaches here then all is success */
        UARTprintf("QSSI : Serial Flash Erased...\n");
        return (1);
    }
}

/* This function sets up SSI External Flash ID Read */
uint32_t SSILibSendReadIDAdvMode(uint32_t ui32Base)
{
   uint32_t ui32Idx, ui32Receive;
   uint32_t ui32MfgID;
   uint32_t ui32DevID;
   uint32_t ui32ReadID[] = {0xAA, 0xAA};
   uint32_t ui32ReadIDFlash;
   uint8_t  ui8InstrReadID[] = {0x90, 0x00, 0x00, 0x00};

   for(ui32Idx = 0;
       ui32Idx < sizeof(ui8InstrReadID) / sizeof(ui8InstrReadID[0]);
       ui32Idx++)
   {
     MAP_SSIDataPut(ui32Base, ui8InstrReadID[ui32Idx]);
   }

   MAP_SSIAdvModeSet(ui32Base,SSI_ADV_MODE_READ_WRITE);
   MAP_SSIDataPut(ui32Base, 0x00);
   MAP_SSIDataGet(ui32Base, &ui32Receive);
   ui32ReadID[0] = ui32Receive;
   MAP_SSIAdvDataPutFrameEnd(ui32Base,0x00);
   MAP_SSIDataGet(ui32Base, &ui32Receive);
   ui32ReadID[1] = ui32Receive;
   ui32MfgID = ui32ReadID[0];
   ui32DevID = ui32ReadID[1];
   ui32ReadIDFlash = ui32MfgID;

   ui32ReadIDFlash = ui32ReadIDFlash << 8;
   ui32ReadIDFlash |= ui32DevID;

   MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);

   return (ui32ReadIDFlash);
}


/* This function sets up SSI External Flash Read of Status Register */
uint8_t SSILibSendReadStatusRegister(uint32_t ui32Base,
                                     uint8_t ui8FlashCommand)
{
    uint32_t ui32Status = 0;
    uint8_t ui8FlashStatus;

    MAP_SSIAdvFrameHoldEnable(ui32Base);
    MAP_SSIDataPut(ui32Base,ui8FlashCommand);
    MAP_SSIAdvModeSet(ui32Base,SSI_ADV_MODE_READ_WRITE);
    MAP_SSIAdvDataPutFrameEnd(ui32Base,DUMMY_BYTE);
    while(MAP_SSIBusy(ui32Base));
    MAP_SSIDataGet(ui32Base, &ui32Status);
    ui8FlashStatus = (uint8_t)ui32Status;
    MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);

    return (ui8FlashStatus);
}


/* This function sets up SSI External Flash Polling of Busy Bit Check */
void SSILibDeviceBusyCheck(uint32_t ui32Base)
{
   uint8_t ui8TempData = 0xFF;

   ui8TempData = SSILibSendReadStatusRegister(ui32Base,
                                              INS_READ_STATUS_REGISTER1);

   while((ui8TempData & FLASH_WIP_BIT) == FLASH_WIP_BIT)
   {
      ui8TempData = SSILibSendReadStatusRegister(ui32Base,
                                                 INS_READ_STATUS_REGISTER1);
   }
}

/* This function sets up SSI External Flash Program in Adv/Quad Mode */
bool SSILibSendPageProgram(uint32_t ui32Base,
                           uint32_t ui32Address,
                           uint8_t ui8FlashCommand)
{
    uint32_t ui32TempAddr;
    uint32_t ui32TempData;
    uint8_t ui8AddrByte1;
    uint8_t ui8AddrByte2;
    uint8_t ui8AddrByte3;

    ui32TempAddr = ui32Address >> 16 ;
    ui8AddrByte1 = (uint8_t)ui32TempAddr;
    ui32TempAddr = ui32Address >> 8 ;
    ui8AddrByte2 = (uint8_t)ui32TempAddr;
    ui8AddrByte3 = (uint8_t)ui32Address;

    /* Check if the Command to Program the Flash is either Advanced or Quad Mode
     * If not then return false. if yes then first set the WEL bit */
    if((ui8FlashCommand == INS_ADV_PAGE_PROGRAM) ||
            (ui8FlashCommand == INS_QUAD_PAGE_PROGRAM))
    {
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
    }
    else
    {
        return (false);
    }

    MAP_SSIAdvFrameHoldEnable(ui32Base);
    MAP_SSIAdvDataPutFrameEnd(ui32Base,INS_WRITE_ENABLE);
    while(MAP_SSIBusy(ui32Base));

    ui32TempData = SSILibSendReadStatusRegister(ui32Base,INS_READ_STATUS_REGISTER1);
    while((ui32TempData & FLASH_WIP_BIT) == FLASH_WIP_BIT)
    {
      ui32TempData = SSILibSendReadStatusRegister(ui32Base,INS_READ_STATUS_REGISTER1);
    }

    /* Now Set the Quad Enable Bit if Quad Mode Program is requested */
    if(ui8FlashCommand == INS_QUAD_PAGE_PROGRAM)
    {
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
        MAP_SSIAdvFrameHoldEnable(ui32Base);
        MAP_SSIDataPut(ui32Base,INS_WRITE_STATUS_REGISTER);
        MAP_SSIDataPut(ui32Base,(FLASH_QE_BIT));
        MAP_SSIAdvDataPutFrameEnd(ui32Base,FLASH_CFG_REG);
        while(MAP_SSIBusy(ui32Base));


        /* Check if Quad Mode setting has been completed */
        ui32TempData = SSILibSendReadStatusRegister(ui32Base,INS_READ_STATUS_REGISTER1);
        while((ui32TempData & FLASH_WIP_BIT) == FLASH_WIP_BIT)
        {
          ui32TempData = SSILibSendReadStatusRegister(ui32Base,INS_READ_STATUS_REGISTER1);
        }

        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
        MAP_SSIAdvFrameHoldEnable(ui32Base);
        MAP_SSIAdvDataPutFrameEnd(ui32Base,INS_WRITE_ENABLE);
        while(MAP_SSIBusy(ui32Base));

        /* Set the WIP bit for Quad Mode Programming and check if
         * it has been completed as well */
        ui32TempData = SSILibSendReadStatusRegister(ui32Base,INS_READ_STATUS_REGISTER1);
        while((ui32TempData & FLASH_WIP_BIT) == FLASH_WIP_BIT)
        {
          ui32TempData = SSILibSendReadStatusRegister(ui32Base,INS_READ_STATUS_REGISTER1);
        }
    }

    if(ui8FlashCommand == INS_ADV_PAGE_PROGRAM)
    {
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
        MAP_SSIAdvFrameHoldEnable(ui32Base);
        MAP_SSIDataPut(ui32Base,ui8FlashCommand);
    }
    if(ui8FlashCommand == INS_QUAD_PAGE_PROGRAM)
    {
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
        MAP_SSIAdvFrameHoldEnable(ui32Base);
        MAP_SSIDataPut(ui32Base,ui8FlashCommand);
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_QUAD_WRITE);
    }
    MAP_SSIDataPut(ui32Base,ui8AddrByte1);
    MAP_SSIDataPut(ui32Base,ui8AddrByte2);
    MAP_SSIDataPut(ui32Base,ui8AddrByte3);

    /* Return Success */
    return (true);
}

/* This function sets up SSI External Flash Read of Data in Advanced/Bi/Quad mode */
bool SSILibSendReadDataAdvBiQuad(uint32_t ui32Base,
                                 uint32_t ui32Address,
                                 uint8_t ui8FlashCommand)
{
    uint32_t ui32TempAddr;
    uint8_t ui8AddrByte1;
    uint8_t ui8AddrByte2;
    uint8_t ui8AddrByte3;

    /* Pre Format the Address */
    ui32TempAddr = ui32Address >> 16 ;
    ui8AddrByte1 = (uint8_t)ui32TempAddr;
    ui32TempAddr = ui32Address >> 8 ;
    ui8AddrByte2 = (uint8_t)ui32TempAddr;
    ui8AddrByte3 = (uint8_t)ui32Address;

    /* Check if Previous operation of Program or Erase has completed */
    SSILibDeviceBusyCheck(ui32Base);

    /* Check if the Command to Program the Flash is either Advanced or
     * Quad Mode If not then return false. if yes then first set the WEL bit */
    if((ui8FlashCommand == INS_ADV_READ_DATA)     ||
            (ui8FlashCommand == INS_BI_READ_DATA) ||
            (ui8FlashCommand == INS_QUAD_READ_DATA))
    {
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
    }
    else
    {
        return (false);
    }

    MAP_SSIAdvFrameHoldEnable(ui32Base);
    MAP_SSIDataPut(ui32Base,ui8FlashCommand);
    MAP_SSIDataPut(ui32Base,ui8AddrByte1);
    MAP_SSIDataPut(ui32Base,ui8AddrByte2);
    MAP_SSIDataPut(ui32Base,ui8AddrByte3);

    if(ui8FlashCommand == INS_ADV_READ_DATA)
    {
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_READ_WRITE);
    }
    if(ui8FlashCommand == INS_BI_READ_DATA)
    {
        MAP_SSIDataPut(ui32Base,DUMMY_BYTE);
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_BI_READ);
    }
    if(ui8FlashCommand == INS_QUAD_READ_DATA)
    {
        MAP_SSIDataPut(ui32Base,DUMMY_BYTE);
        MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_QUAD_READ);
    }

    return (true);
}

/* This function sets up SSI External Flash Erase */
void SSILibSendEraseCommand (uint32_t ui32Base,
                             uint32_t ui32Address,
                             uint8_t  ui8FlashCommand)
{
    uint32_t ui32TempAddr;
    uint8_t ui8AddrByte1;
    uint8_t ui8AddrByte2;
    uint8_t ui8AddrByte3;

    ui32TempAddr = ui32Address >> 16 ;

    ui8AddrByte1 = (uint8_t)ui32TempAddr;
    ui32TempAddr = ui32Address >> 8 ;
    ui8AddrByte2 = (uint8_t)ui32TempAddr;
    ui8AddrByte3 = (uint8_t)ui32Address;

    MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
    MAP_SSIAdvFrameHoldEnable(ui32Base);
    MAP_SSIAdvDataPutFrameEnd(ui32Base,INS_WRITE_ENABLE);
    while(MAP_SSIBusy(ui32Base));

    /* Check if Previous operation of Program or Erase has completed */
    SSILibDeviceBusyCheck(ui32Base);

    MAP_SSIAdvModeSet(SSI1_BASE,SSI_ADV_MODE_WRITE);
    MAP_SSIAdvFrameHoldEnable(ui32Base);
    MAP_SSIDataPut(ui32Base,ui8FlashCommand);
    MAP_SSIDataPut(ui32Base,ui8AddrByte1);
    MAP_SSIDataPut(ui32Base,ui8AddrByte2);
    MAP_SSIAdvDataPutFrameEnd(ui32Base,ui8AddrByte3);
    while(MAP_SSIBusy(ui32Base));

    /* Wait till the erase is completed */
    SSILibDeviceBusyCheck(ui32Base);
}
