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
 *  ======== main_freertos.c ========
 */

/* Standard header files */
#include <stdbool.h>
#include <mqueue.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>

/* Driver header files */
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include <ti/drivers/Board.h>
#include "board_funcs.h"
#include "command_task.h"

extern void ti_ndk_config_Global_startupFxn();
extern void MSP_EXP432E401Y_initEMAC(void);

mqd_t               mqCmdQueueBlock;
mqd_t               mqCmdQueueNoBlock;
mqd_t               mqCloudQueueBlock;
mqd_t               mqCloudQueueNoBlock;

/*
 *  ======== main ========
 */
int main(void)
{
    struct mq_attr      attr;
    int                 retc;

    /* Call driver init functions */
    Board_init();

    /* Initialize GPIO */
    GPIO_init();

    /* Configure UART0 */
    ConfigureUART();

    /* Configure ADC0 */
    ConfigureADC0();

    /* Configure the buttons */
    ConfigureButtons();

    /* Initialize EEPROM to store the CIK */
    NVSInit();

    ti_ndk_config_Global_startupFxn();

    retc = CommandTaskInit();
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    /* Create RTOS Queue */
    attr.mq_maxmsg = 3;
    attr.mq_msgsize = 132;
    attr.mq_flags = 0;

    /* Create a message queue that will receive data in the cloud task in a
     * non-blocking fashion */
    mqCloudQueueBlock = mq_open("CloudQueue", O_RDWR | O_CREAT, 0664, &attr);
    if (mqCloudQueueBlock == ((mqd_t)(-1))) {
        /* mq_open() failed */
        while (1);
    }
    /* Create a message queue that will send data to cloud task in a
     * non-blocking fashion */
    mqCloudQueueNoBlock = mq_open("CloudQueue", O_RDWR | O_CREAT | O_NONBLOCK,
                              0664, &attr);
    if (mqCloudQueueNoBlock == ((mqd_t)(-1))) {
        /* mq_open() failed */
        while (1);
    }

    /* Increasing the message size to 4 to accommodate debug messages from
     * serviceReportHook during initialization. */
    attr.mq_maxmsg = 4;

    /* Create a message queue that will receive data in the command task */
    mqCmdQueueBlock = mq_open("CmdQueue", O_RDWR | O_CREAT, 0664, &attr);
    if (mqCmdQueueBlock == ((mqd_t)(-1))) {
        /* mq_open() failed */
        while (1);
    }

    /* Create a message queue that will send data to command task in a
     * non-blocking fashion */
    mqCmdQueueNoBlock = mq_open("CmdQueue", O_RDWR | O_CREAT | O_NONBLOCK,
                                0664, &attr);
    if (mqCmdQueueNoBlock == ((mqd_t)(-1))) {
        /* mq_open() failed */
        while (1);
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return (0);
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    /* Handle Memory Allocation Errors */
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    /* Handle FreeRTOS Stack Overflow */
    while(1)
    {
    }
}
