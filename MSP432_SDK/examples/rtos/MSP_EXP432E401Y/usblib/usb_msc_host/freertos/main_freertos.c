/*
 *  ======== main_freertos.c ========
 */
#include <stdbool.h>
#include <stdint.h>

#ifdef __ICCARM__
#include <DLib_Threads.h>
#endif

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "ti_drivers_config.h"
#include "ti_usblib_config.h"

/* USB Reference Module Header file */
#include "USBMSCH.h"

#define THREADSTACKSIZE   1024

extern void *mscHostFxn(void *arg0);

int main(void)
{
    pthread_t         thread;
    pthread_attr_t    attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* initialize the system locks */
#ifdef __ICCARM__
    __iar_Initlocks();
#endif

    /* Call board init functions */
    Board_init();
    GPIO_init();
    MSP_EXP432E401Y_initUSB(MSP_EXP432E401Y_USBHOST);
	
    /* Set priority and stack size attributes */
    pthread_attr_init(&attrs);
    priParam.sched_priority = 1;

    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    pthread_attr_setschedparam(&attrs, &priParam);

    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    retc = pthread_create(&thread, &attrs, mscHostFxn, (void *)MSP_EXP432E401Y_USBHOST);
    if (retc != 0) {
        /* pthread_create() failed */
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
    // Handle Memory Allocation Errors
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
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
