/*
 * init.c
 *
 * Authors: Dario Jimenez
 *
 * @file contains hub unit's main method and initialization
 */

/** includes **/

/* standard library */
#include <stdint.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <guadaloop/lib/sensors/Front_Hub_Variables.h>
#include <guadaloop/lib/sensors/queueData.h>
/* TI MSP432 SDK includes */
#include <ti/drivers/Board.h>

/** constants and macros **/
#define THREADSTACKSIZE   1024  /* Stack size in bytes */

/*
 * enum for hub unit tasks
 */
typedef enum hubunit_task {
    VCU_RECEIVE,
    VCU_SEND,
    //TODO: add any other tasks here
}hubunit_task_t;

/*
 * @brief creates any given task for the hub unit
 *
 * @param task the task to create
 */
static void create_task(hubunit_task_t task) {
    //TODO: add task creation here
    switch(task) {
        case VCU_RECEIVE: {
//            BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
//             const char * const pcName,
//             uint16_t usStackDepth,
//            void *pvParameters,
//            UBaseType_t uxPriority,
//             TaskHandle_t *pxCreatedTask );
            break;
        }

        default:{
            break;
        }
    }
}

/*
 * @brief create all tasks necessary for hub unit
 */
static void create_tasks(void) {
    //TODO
}
/*
 *  ======== main ========
 */

UBaseType_t length;
UBaseType_t size;


int main(void)
{
    length = 0; //Replace with real size
    size = sizeof(queueData);
    xQueue = xQueueCreate(length, size);


    /* init board for msp432 sdk */
    Board_init();

    /* init gpio, interrupts, can, etc. here */

    /* create tasks */
    create_tasks();

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return (0);
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
