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
 *  ======== comparatorinterrupt.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/Comparator.h>
#include <ti/drivers/GPIO.h>

/* Board Header file */
#include "ti_drivers_config.h"

/*
 * ======= comparatorCallback ========
 */
void comparatorCallback(Comparator_Handle handle, int_fast16_t status)
{
    /* Toggle Board LED0 to match the comparator output */
    if(Comparator_getLevel(handle) == Comparator_OUTPUT_HIGH)
    {
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
    }
    else
    {
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Comparator_Handle chandle;
    Comparator_Params cparams;

    /* Initialize GPIO */
    GPIO_init();
    /* Set GPIO Configuration */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    /* Start with LED off */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);

    /* Initialize Comparator */
    Comparator_init();
    Comparator_Params_init(&cparams);

    /* Assign comparatorCallback as the callback for this instance */
    cparams.callbackFxn = (Comparator_CallBackFxn)&comparatorCallback;
    /* Invert output because input voltage is on negative terminal */
    cparams.outputPolarity = Comparator_OUTPUT_INVERTED;
    /* Set to trigger an interrupt on either a rising or falling edge */
    cparams.interruptLevel = Comparator_INTERRUPT_BOTH;

    /* Open a comparator instance */
    chandle = Comparator_open(CONFIG_COMPARATOR_0, &cparams);
    if(chandle == NULL)
    {
        while(1) {}
    }

    /* Start the comparator */
    Comparator_start(chandle);

    /* Loop forever, operating in interrupt context */
    while(1);
}
