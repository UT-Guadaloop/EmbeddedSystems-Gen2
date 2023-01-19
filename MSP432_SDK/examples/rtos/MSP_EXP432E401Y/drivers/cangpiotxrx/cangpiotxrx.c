/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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
 *  ======== cangpiotxrx.c ========
 */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>

/* Driver Header files */
#include <ti/drivers/CAN.h>
#include <ti/drivers/GPIO.h>
#include <ti/display/Display.h>

/* Example/Board Header files */
#include "ti_drivers_config.h"

#define THREADSTACKSIZE (1024)

static CAN_Handle can;
static Display_Handle display;
static uint32_t framesSent;

/* Semaphore to block TX UART display until CONFIG_GPIO_BUTTON_1 is pressed */
sem_t txSem;

/*
 * ======== gpioButtonFxn ========
 * Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_1.
 */
void gpioButtonFxn(uint_least8_t index)
{
    sem_post(&txSem);
}

/*
 * ======== txThread ========
 * Transmits a CAN frame every time CONFIG_GPIO_BUTTON_1 is pressed.
 */
void *txThread(void *arg0)
{
    while(1) {
        sem_wait(&txSem);
        CAN_Frame canFrame = {0};
        canFrame.id = framesSent++;
        canFrame.dlc = 1;
        canFrame.data[0] = 0;
        CAN_write(can, &canFrame, sizeof(canFrame));
        Display_print1(display, 0, 0, "Button pressed. Frame ID sent: 0x%3x",
            canFrame.id);
    }
}

/*
 * ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    pthread_t           thread0;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;
    int32_t             status;

    /* Keep track of frames sent */
    framesSent = 0;

    /* Call driver init functions */
    GPIO_init();
    CAN_init();
    Display_init();

    /*
     * Open CAN instance.
     * CAN_Params filterID and filterMask determine what messages to receive.
     */
    CAN_Params canParams;
    CAN_Params_init(&canParams);
    canParams.filterID = 0x001;
    canParams.filterMask = 0x001;
    can = CAN_open(CONFIG_CAN_0, &canParams);

    /* Open Display instance for UART */
    Display_Params displayParams;
    Display_Params_init(&displayParams);
    display = Display_open(Display_Type_UART, &displayParams);

    Display_print0(display, 0, 0, "==Press USR_SW1 to transmit a message==");

    /* Configure the LED and BUTTON pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_1,
        GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /*
     * Create synchronization semaphore; the TX Display will wait on this
     * semaphore until GPIO button is pressed.
     */
    status = sem_init(&txSem, 0, 0);
    if (status != 0) {
        Display_printf(display, 0, 0, "Error creating txSem\n");
        while(1);
    }

    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_1);

    /* Create application threads */
    pthread_attr_init(&attrs);
    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);

    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    retc |= pthread_create(&thread0, &attrs, txThread, NULL);
    if (retc != 0) {
            /* pthread_create() failed */
            while (1);
    }

    /*
     * Toggles Board_GPIO_LED0 every time a CAN frame is received.
     * Also prints out the received CAN frame ID to UART display.
     * CAN_Params filterID, filterMask control how often CAN_read is completed.
     */
    CAN_Frame canFrame = {0};
    while(1) {
        CAN_read(can, &canFrame, sizeof(canFrame));
        Display_print1(display, 0, 0,
                       "Message received. Frame ID received: 0x%3x",
                       canFrame.id);
        GPIO_toggle(CONFIG_GPIO_LED_0);
    }
}
