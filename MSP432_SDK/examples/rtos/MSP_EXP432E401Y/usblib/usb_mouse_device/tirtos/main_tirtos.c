/*
 *  ======== main_tirtos.c ========
 */
#include <stdbool.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>  

/* Example/Board Header files */
#include "ti_drivers_config.h"
#include "ti_usblib_config.h"

/* USB Reference Module Header file */
#include "USBMD.h"

#define THREADSTACKSIZE   768

extern void *mouseFxn(void *arg0);

/*
 * The following (weak) function definition is needed in applications
 * that do *not* use the NDK TCP/IP stack:
 */
#if defined(__IAR_SYSTEMS_ICC__)
__weak void NDK_hookInit(int32_t id) {}
#elif defined(__GNUC__) && !defined(__ti__)
void __attribute__((weak)) NDK_hookInit(int32_t id) {}
#else
#pragma WEAK (NDK_hookInit)
void NDK_hookInit(int32_t id) {}
#endif

int main(void)
{
    
    pthread_t         thread;
    pthread_attr_t    attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Call board init functions */
    Board_init();
    GPIO_init();
    MSP_EXP432E401Y_initUSB(MSP_EXP432E401Y_USBDEVICE);

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

    retc = pthread_create(&thread, &attrs, mouseFxn, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }

    /* Turn on user LED */
    GPIO_write(CONFIG_LED_0_GPIO, CONFIG_LED_ON);

    USBMD_init(MSP_EXP432E401Y_USBDEVICE);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
