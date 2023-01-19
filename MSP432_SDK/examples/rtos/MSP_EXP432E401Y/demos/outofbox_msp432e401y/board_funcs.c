/*
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
 */
 
 /*
 *  ======== board_funcs.c ========
 */

/* Standard header files */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Driver header files */
#include <ti/devices/msp432e4/inc/msp432.h>
#include <ti/devices/msp432e4/driverlib/inc/hw_adc.h>
#include <ti/devices/msp432e4/driverlib/inc/hw_uart.h>
#include <ti/devices/msp432e4/driverlib/adc.h>
#include <ti/devices/msp432e4/driverlib/eeprom.h>
#include <ti/devices/msp432e4/driverlib/flash.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/uart.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Board Header file */
#include "ti_drivers_config.h"
#include "board_funcs.h"
#include "cloud_task.h"

/* Global counters that hold the count of buton presses since reset */
uint32_t g_ui32SW1 = 0;
uint32_t g_ui32SW2 = 0;

/*
 *  ======== gpioSWFxn1 ========
 * Callback function for the GPIO interrupt on CONFIG_BUTTON0
 */
void gpioSWFxn1(uint_least8_t ui32Index)
{
    g_ui32SW1++;
}

/*
 *  ======== gpioSWFxn1 ========
 * Callback function for the GPIO interrupt on CONFIG_BUTTON10
 */
void gpioSWFxn2(uint_least8_t ui32Index)
{
    g_ui32SW2++;
}

/*
 *  ======== ConfigureADC0 ========
 * Enables and configures ADC0 to read the internal temperature sensor into
 * sample sequencer 3.
 */
void
ConfigureADC0(void)
{
    /* Enable clock to ADC0. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    /* Configure ADC0 Sample Sequencer 3 for processor trigger operation. */
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    /* Configure ADC0 sequencer 3 for a single sample of the temperature
     * sensor. */
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_TS | ADC_CTL_IE |
                             ADC_CTL_END);

    /* Enable the sequencer. */
    ADCSequenceEnable(ADC0_BASE, 3);

    /* Clear the interrupt bit for sequencer 3 to make sure it is not set
    * before the first sample is taken. */
    ADCIntClear(ADC0_BASE, 3);
}

/*
 *  ======== ConfigureButtons ========
 * This function registers the callback functions to be called when a button is
 * pressed and enables the interrupts
 */
void
ConfigureButtons(void)
{
    /* Install Button callback. */
    GPIO_setCallback(CONFIG_GPIO_BUTTON0, gpioSWFxn1);
    GPIO_setCallback(CONFIG_GPIO_BUTTON1, gpioSWFxn2);

    /* Enable interrupts. */
    GPIO_enableInt(CONFIG_GPIO_BUTTON0);
    GPIO_enableInt(CONFIG_GPIO_BUTTON1);
}

/*
 *  ======== ConfigureUART ========
 * This function initializes the UART0 and the Associated GPIOs
 */
void ConfigureUART(void)
{
    /* Enable and configure the peripherals used by the uart. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Initialize the UART driver */
    UART_init();
}

/*
 *  ======== ReadButtons ========
 * Returns the number of times SW1 and SW2 were pressed.
 */
void
ReadButtons(uint32_t *buttons)
{
    *(buttons) = g_ui32SW1;
    *(++buttons) = g_ui32SW2;
}

/*
 *  ======== ReadInternalTemp ========
 * Calculates the internal junction temperature and returns this value.
 */
uint16_t
ReadInternalTemp(void)
{
    uint32_t ui32Temperature;
    uint32_t ui32ADCValue;

    /* Take a temperature reading with the ADC. */
    ADCProcessorTrigger(ADC0_BASE, 3);

    /* Wait for the ADC to finish taking the sample */
    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    /* Clear the interrupt */
    ADCIntClear(ADC0_BASE, 3);

    /* Read the analog voltage measurement. */
    ADCSequenceDataGet(ADC0_BASE, 3, &ui32ADCValue);

    /* Convert the measurement to degrees Celcius. */
    ui32Temperature = ((1475 * 4096) - (2250 * ui32ADCValue)) / 40960;

    return (ui32Temperature);
}

/*
 *  ======== GetMacAddress ========
 * Get MAC address of the board from FLASH User Registers.
 */
bool
GetMacAddress(char *pcMACAddress, uint32_t ui32MACLen)
{
    uint32_t ulUser0, ulUser1;

    /* Get the MAC address from User Registers. */
    FlashUserGet(&ulUser0, &ulUser1);

    /* Check if MAC address is present on the board. */
    if ((ulUser0 == 0xffffffff) && (ulUser1 == 0xffffffff))
    {
        /* If not present, return error. */
        return false;
    }

    /* Convert the 24/24 split MAC address from NV memory into a 32/16 split
    * MAC address, then convert to string in hex format. */
    snprintf(pcMACAddress, ui32MACLen, "%02x%02x%02x%02x%02x%02x",
             (char)((ulUser0 >>  0) & 0xff), (char)((ulUser0 >>  8) & 0xff),
             (char)((ulUser0 >> 16) & 0xff), (char)((ulUser1 >>  0) & 0xff),
             (char)((ulUser1 >>  8) & 0xff), (char)((ulUser1 >> 16) & 0xff));

    /* Return success. */
    return true;
}

/*
 *  ======== NVSInit ========
 * Initialize the Non volatile storage.
 */
void
NVSInit(void)
{
    /* Enable the EEPROM peripheral. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

    /* Initialize the EEPROM */
    EEPROMInit();
}

/*
 *  ======== GetPIDEEPROM ========
 * Get/Read the PID value from EEPROM.
 */
bool
GetPIDEEPROM(char *pcPIDBuf)
{
    /* Read CIK from the EEPROM. */
    EEPROMRead((uint32_t *)pcPIDBuf, (uint32_t)(EXOSITE_PID_OFFSET),
               (uint32_t)EXOSITE_PID_LENGTH);

    /* Add a trailing 0 to enable working with string functions. */
    pcPIDBuf[EXOSITE_PID_LENGTH] = '\0';

    /* Return Success. */
    return true;

}

/*
 *  ======== GetCIKEEPROM ========
 * Get/Read the CIK value from EEPROM.
 */
bool
GetCIKEEPROM(char *pcProvBuf)
{
    /* Read CIK from the EEPROM. */
    EEPROMRead((uint32_t *)pcProvBuf, (uint32_t)(EXOSITE_CIK_OFFSET),
               (uint32_t)EXOSITE_CIK_LENGTH);

    /* Add a trailing 0 to enable working with string functions. */
    pcProvBuf[EXOSITE_CIK_LENGTH] = '\0';

    /* Return Success. */
    return true;
}

/*
 *  ======== SavePIDEEPROM ========
 * Save/Write the CIK value to EEPROM.
 */
bool
SavePIDEEPROM(char *pcPIDBuf)
{

    uint32_t ui32Len;

    /* Get the length of the buffer. */
    ui32Len = strlen(pcPIDBuf);
    pcPIDBuf[ui32Len] = '\0';

    /* Check if the buffer length is the expected length. */
    if(ui32Len > EXOSITE_PID_LENGTH)
    {
        /* Buffer length is not the expected length. */
        return false;
    }

    /* Write CIK to the EEPROM. */
    EEPROMProgram((uint32_t *)pcPIDBuf, (uint32_t)(EXOSITE_PID_OFFSET),
                  (uint32_t)EXOSITE_PID_LENGTH);

    /* Return Success. */
    return true;
}

/*
 *  ======== SaveCIKEEPROM ========
 * Save/Write the CIK value to EEPROM.
 */
bool
SaveCIKEEPROM(char *pcProvBuf)
{
    uint32_t ui32Len;

    /* Get the length of the buffer. */
    ui32Len = strlen(pcProvBuf);

    /* Check if the buffer length is the expected length. */
    if(ui32Len != EXOSITE_CIK_LENGTH)
    {
        /* Buffer length is not the expected length */
        return false;
    }

    /* Write CIK to the EEPROM. */
    EEPROMProgram((uint32_t *)pcProvBuf, (uint32_t)(EXOSITE_CIK_OFFSET),
                  (uint32_t)ui32Len);

    /* Return Success. */
    return true;
}

/*
 *  ======== EraseEEPROM ========
 * Erase EEPROM.  This will erase everything including the CIK.
 */
void
EraseEEPROM(void)
{
    /* Perform a mass erase on the EEPROM. */
    EEPROMMassErase();
}

/*
 *  ======== CheckBufZero ========
 * Check if a buffer is filled with zero.  If buffer is filled with zero's
 * zero is returned.  Else a non zero number is returned.
 */
int32_t
CheckBufZero(char * pcBuf, uint32_t ui32Size)
{
    uint32_t ui32Sum = 0;
    uint32_t ui32Ind;

    /* Iterate and or each byte in the buffer with the previous accumulation */
    for (ui32Ind = 0; ui32Ind < ui32Size; ui32Ind++)
    {
        ui32Sum |= pcBuf[ui32Ind];
    }

    return (ui32Sum);
}
