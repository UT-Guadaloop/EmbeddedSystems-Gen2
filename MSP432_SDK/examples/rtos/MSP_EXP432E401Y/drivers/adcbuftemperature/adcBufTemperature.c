/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
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
 *  ======== adcBufTemperature.c ========
 */
#include <stdint.h>
#include <stdio.h>

/* Driver Header files */
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/GPIO.h>

#include <semaphore.h>

/* Display Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>

/* For sleep() */
#include <unistd.h>

/* Board Header file */
#include "ti_drivers_config.h"

#define ADCSAMPLESIZE    (20)

uint16_t sampleBufferOne[ADCSAMPLESIZE];
float CBuffer[ADCSAMPLESIZE];
float avgTemperature;

/* Driver handles shared between the task and the callback function */
ADCBuf_Handle adcBuf;
ADCBuf_Conversion continuousConversion;

/* Display Driver Handle */
Display_Handle displayHandle;

/* ADCBuf semaphore */
sem_t adcbufSem;

/*
 * This function is called whenever a buffer is full.
 * The content of the buffer is then averaged and converted into degrees Celsius format
 * and sent to the PC via the UART.
 */
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel, int_fast16_t status) {
    uint_fast16_t i;
    uint16_t *rawTemperatureBuf = (uint16_t *) completedADCBuffer;

    avgTemperature = 0;

    /* Calculate average temperature */
    for (i = 0; i < ADCSAMPLESIZE; i++) {
        avgTemperature += rawTemperatureBuf[i];
    }
    avgTemperature = avgTemperature/ADCSAMPLESIZE;

    /* Convert ADC value to Celsius */
    avgTemperature = (1475*4096 - (75 * 33 * avgTemperature))/ 40960;

    /* post adcbuf semaphore */
    sem_post(&adcbufSem);
}

/*
 *  ======== gpioButtonFxn0 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_0.
 */
void gpioButtonFxn0(uint_least8_t index)
{
    /* Start converting. */
    if (ADCBuf_convert(adcBuf, &continuousConversion, 1) !=
        ADCBuf_STATUS_SUCCESS) {
        /* Did not start conversion process correctly. */
        while(1);
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    ADCBuf_Params adcBufParams;
    Display_Params displayParams;

    /* Call driver init functions */
    ADCBuf_init();
    GPIO_init();
    Display_init();
    int32_t         status;

    /* Configure & open Display driver */
    Display_Params_init(&displayParams);
    displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    displayHandle = Display_open(Display_Type_UART, &displayParams);
    if (displayHandle == NULL) {
        while (1);
    }

    /* Configure button pins and install Button callback */
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    status = sem_init(&adcbufSem, 0, 0);
    if (status != 0) {
        Display_printf(displayHandle, 0, 0, "Error creating adcbufSem\n");
        while(1);
    }

    Display_printf(displayHandle, 0, 0, "Starting the ADCBuf temperature example");

    /* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_ONE_SHOT;
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.samplingFrequency = 200;

    adcBuf = ADCBuf_open(CONFIG_ADCBUF_0, &adcBufParams);

    if (!adcBuf){
        /* AdcBuf did not open correctly. */
        while(1);
    }

    /* Configure the conversion struct */
    continuousConversion.arg = NULL;
    continuousConversion.adcChannel = CONFIG_ADCBUF_0_CHANNEL_0;
    continuousConversion.sampleBuffer = sampleBufferOne;
    continuousConversion.sampleBufferTwo = sampleBufferOne;
    continuousConversion.samplesRequestedCount = ADCSAMPLESIZE;

    /*
     * Go to sleep in the foreground thread forever. The data will be collected
     * and transfered in the background thread
     */
    while(1) {
        /* Wait for semaphore and print average temperature */
        sem_wait(&adcbufSem);
       /* Print a message with average temperature */
        Display_printf(displayHandle, 0, 0, "The average temperature is %.3fC",
         avgTemperature);
    }

}
