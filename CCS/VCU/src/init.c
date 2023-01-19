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

/* TI MSP432 SDK includes */
#include <ti/drivers/Board.h>

/** constants and macros **/
#define THREADSTACKSIZE   1024  /* Stack size in bytes */

/*
 * enum for hub unit tasks
 */
typedef enum vcu_task {
    HUBUNIT_SEND,
    //TODO: add any other tasks here
}vcu_task_t;

/*
 * @brief creates any given task for the hub unit
 *
 * @param task the task to create
 */
static void create_task(vcu_task_t task) {
    //TODO: add task creation here
    switch(task) {
        case HUBUNIT_SEND: {
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
int main(void)
{
    /* init board for msp432 sdk */
    Board_init();

    /* init gpio, interrupts, can, etc. here */
    //TODO

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
