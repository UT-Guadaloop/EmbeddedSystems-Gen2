/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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

#include <stdbool.h>
#include <stdint.h>
#include "touch.h"
#include <ti/grlib/grlib.h>
#include <ti/devices/msp432e4/driverlib/adc.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/timer.h>
#include <ti/devices/msp432e4/driverlib/types.h>
#include <ti/devices/msp432e4/inc/msp432.h>

#define ADC_CMSIS(x) ((ADC0_Type *) x)
#define GPIO_CMSIS(x) ((GPIO_Type *) x)
#define TIMER_CMSIS(x) ((TIMER0_Type *) x)

/* The most recent raw ADC reading for the X position on the screen. */
volatile int16_t touchX;

/* The most recent raw ADC reading for the Y position on the screen. */
volatile int16_t touchY;

/* The minimum raw reading that should be considered valid press. */
int16_t touchMin = TOUCH_MIN;

/* A pointer to the function to receive messages from the touch screen driver
   when events occur on the touch screen (debounced presses, movement while
   pressed, and debounced releases). */
static int32_t (*touchHandler)(uint32_t message, int32_t x,
        int32_t y);

/* The current state of the touch screen debouncer.  When zero, the pen is up.
   When three, the pen is down.  When one or two, the pen is transitioning from
   one state to the other. */
static uint8_t state = 0;

/* The queue of debounced pen positions.  This is used to slightly delay the
   returned pen positions, so that the pen positions that occur while the pen
   is being raised are not send to the application. */
static int16_t samples[8];

/* The count of pen positions in samples.  When negative, the buffer is
   being pre-filled as a result of a detected pen down event. */
static int8_t index = 0;

/* The current state of the touch screen driver's state machine.  This is used
   to cycle the touch screen interface through the powering sequence required
   to read the two axes of the surface. */
static uint32_t touchState;

/* Touchscreen calibration parameters.  Screen orientation is a build time
   selection. */
const int32_t touchParams[7] = {
#ifdef PORTRAIT
    3840,                       // M0
    318720,                     // M1
    -297763200,                 // M2
    328576,                     // M3
    -8896,                      // M4
    -164591232,                 // M5
    3100080,                    // M6
#endif
#ifdef LANDSCAPE
    328192,                     // M0
    -4352,                      // M1
    -178717056,                 // M2
    1488,                       // M3
    -314592,                    // M4
    1012670064,                 // M5
    3055164,                    // M6
#endif
#ifdef PORTRAIT_FLIP
    1728,                       // M0
    -321696,                    // M1
    1034304336,                 // M2
    -325440,                    // M3
    1600,                       // M4
    1161009600,                 // M5
    3098070,                    // M6
#endif
#ifdef LANDSCAPE_FLIP
    -326400,                    // M0
    -1024,                      // M1
    1155718720,                 // M2
    3768,                       // M3
    312024,                     // M4
    -299081088,                 // M5
    3013754,                    // M6
#endif
};

/*
 *  ======== TouchScreenDebouncer ========
 *  Debounces presses of the touch screen.
 */
static void TouchScreenDebouncer(void)
{
    int32_t x, y, temp;

    /* Convert the ADC readings into pixel values on the screen. */
    x = touchX;
    y = touchY;
    temp = (((x * touchParams[0]) +
        (y * touchParams[1]) + touchParams[2]) /
               touchParams[6]);
    y = (((x * touchParams[3]) +
            (y * touchParams[4]) + touchParams[5]) /
            touchParams[6]);
    x = temp;

    /* See if the touch screen is being touched. */
    if((touchX < touchMin) || (touchY < touchMin)) {
        /* If there are no valid values yet then ignore this state. */
        if((state & 0x80) == 0) {
            state = 0;
        }

        /* See if the pen is not up right now. */
        if(state != 0x00) {
            /* Decrement the state count. */
            state--;

            /* See if the pen has been detected as up three times in a row. */
            if(state == 0x80) {
                /* Indicate that the pen is up. */
                state = 0x00;

                /* See if there is a touch screen event handler. */
                if(touchHandler) {
                    /* If we got caught pre-filling the values, just return the
                       first valid value as a press and release.  If this is
                       not done there is a perceived miss of a press event. */
                    if(index < 0) {
                        touchHandler(MSG_PTR_DOWN, samples[0],
                            samples[1]);
                        index = 0;
                    }

                    /* Send the pen up message to the touch screen event
                       handler. */
                    touchHandler(MSG_PTR_UP, samples[index],
                        samples[index + 1]);
                }
            }
        }
    }
    else {
        /* If the state was counting down above then fall back to the idle
           state and start waiting for new values. */
        if((state & 0x80) && (state != 0x83)) {
            /* Restart the release count down. */
            state = 0x83;
        }

        /* See if the pen is not down right now. */
        if(state != 0x83) {
            /* Increment the state count. */
            state++;

            /* See if the pen has been detected as down three times in a row. */
            if(state == 0x03) {
                /* Indicate that the pen is down. */
                state = 0x83;

                /* Set the index to -8, so that the next 3 samples are stored
                   into the sample buffer before sending anything back to the
                   touch screen event handler. */
                index = -8;

                /* Store this sample into the sample buffer. */
                samples[0] = x;
                samples[1] = y;
            }
        }
        else {
            /* See if the sample buffer pre-fill has completed. */
            if(index == -2) {
                /* See if there is a touch screen event handler. */
                if(touchHandler) {
                    /* Send the pen down message to the touch screen event
                       handler. */
                    touchHandler(MSG_PTR_DOWN, samples[0],
                                   samples[1]);
                }

                /* Store this sample into the sample buffer. */
                samples[0] = x;
                samples[1] = y;

                /* Set the index to the next sample to send. */
                index = 2;
            }

            /* Otherwise, see if the sample buffer pre-fill is in progress. */
            else if(index < 0) {
                /* Store this sample into the sample buffer. */
                samples[index + 10] = x;
                samples[index + 11] = y;

                /* Increment the index. */
                index += 2;
            }

            /* Otherwise, the sample buffer is full. */
            else {
                /* See if there is a touch screen event handler. */
                if(touchHandler) {
                    /* Send the pen move message to the touch screen event
                       handler. */
                    touchHandler(MSG_PTR_MOVE, samples[index],
                        samples[index + 1]);
                }

                /* Store this sample into the sample buffer. */
                samples[index] = x;
                samples[index + 1] = y;

                /* Increment the index. */
                index = (index + 2) & 7;
            }
        }
    }
}

/*
 *  ======== ADC0SS3_IRQHandler ========
 *  Handles the ADC interrupt for the touch screen.
 */
void ADC0SS3_IRQHandler(void)
{
    /* Clear the ADC sample sequence interrupt. */
   ADC_CMSIS(ADC0_BASE)->ISC = ADC_ISC_IN3;

    /* Determine what to do based on the current state of the state machine. */
    switch(touchState) {
        /* The new sample is an X axis sample that should be discarded. */
        case TS_STATE_SKIP_X: {
            /* Read and throw away the ADC sample. */
            ADC_CMSIS(ADC0_BASE)->SSFIFO3;

            /* Set the analog mode select for the YP pin. */
            GPIO_CMSIS(TS_YP_BASE)->AMSEL |= TS_YP_PIN;

            /* Configure the Y axis touch layer pins as inputs. */
            GPIO_CMSIS(TS_YP_BASE)->DIR &= ~TS_YP_PIN;
            GPIO_CMSIS(TS_YN_BASE)->DIR &= ~TS_YN_PIN;

            /* The next sample will be a valid X axis sample. */
            touchState = TS_STATE_READ_X;

            /* This state has been handled. */
            break;
        }

        /* The new sample is an X axis sample that should be processed. */
        case TS_STATE_READ_X: {
            /* Read the raw ADC sample. */
            touchX = ADC_CMSIS(ADC0_BASE)->SSFIFO3;

            /* Clear the analog mode select for the YP pin. */
            GPIO_CMSIS(TS_YP_BASE)->AMSEL &= ~TS_YP_PIN;

            /* Configure the X and Y axis touch layers as outputs. */
            GPIO_CMSIS(TS_XP_BASE)->DIR |= TS_XP_PIN;
            GPIO_CMSIS(TS_XN_BASE)->DIR |= TS_XN_PIN;
            GPIO_CMSIS(TS_YP_BASE)->DIR |= TS_YP_PIN;
            GPIO_CMSIS(TS_YN_BASE)->DIR |= TS_YN_PIN;

            /* Drive the positive side of the Y axis touch layer with VDD and
               the negative side with GND.  Also, drive both sides of the X
               axis layer with GND to discharge any residual voltage (so that
               a no-touch condition can be properly detected). */
            GPIO_CMSIS(TS_XP_BASE)->DATA &= ~TS_XP_PIN;
            GPIO_CMSIS(TS_XN_BASE)->DATA &= ~TS_XN_PIN;
            GPIO_CMSIS(TS_YP_BASE)->DATA |= TS_YP_PIN;
            GPIO_CMSIS(TS_YN_BASE)->DATA &= ~TS_YN_PIN;

            /* Configure the sample sequence to capture the X axis value. */
            ADC_CMSIS(ADC0_BASE)->SSMUX3 = TS_XP_ADC;

            /* The next sample will be an invalid Y axis sample. */
            touchState = TS_STATE_SKIP_Y;

            /* This state has been handled. */
            break;
        }

        /* The new sample is a Y axis sample that should be discarded. */
        case TS_STATE_SKIP_Y: {
            /* Read and throw away the ADC sample. */
            ADC_CMSIS(ADC0_BASE)->SSFIFO3;

            /* Set the analog mode select for the XP pin. */
            GPIO_CMSIS(TS_XP_BASE)->AMSEL |= TS_XP_PIN;

            /* Configure the X axis touch layer pins as inputs. */
            GPIO_CMSIS(TS_XP_BASE)->DIR &= ~TS_XP_PIN;
            GPIO_CMSIS(TS_XN_BASE)->DIR &= ~TS_XN_PIN;

            /* The next sample will be a valid Y axis sample. */
            touchState = TS_STATE_READ_Y;

            /* This state has been handled. */
            break;
        }

        /* The new sample is a Y axis sample that should be processed. */
        case TS_STATE_READ_Y: {
            /* Read the raw ADC sample. */
            touchY = ADC_CMSIS(ADC0_BASE)->SSFIFO3;
            /* The next configuration is the same as the initial configuration.
               Therefore, fall through into the initialization state to avoid
               duplicating the code. */
        }

        /* The state machine is in its initial state */
        case TS_STATE_INIT: {
            /* Clear the analog mode select for the XP pin. */
            GPIO_CMSIS(TS_XP_BASE)->AMSEL &= ~TS_XP_PIN;

            /* Configure the X and Y axis touch layers as outputs. */
            GPIO_CMSIS(TS_XP_BASE)->DIR |= TS_XP_PIN;
            GPIO_CMSIS(TS_XN_BASE)->DIR |= TS_XN_PIN;
            GPIO_CMSIS(TS_YP_BASE)->DIR |= TS_YP_PIN;
            GPIO_CMSIS(TS_YN_BASE)->DIR |= TS_YN_PIN;

            /* Drive one side of the X axis touch layer with VDD and the other
               with GND.  Also, drive both sides of the Y axis layer with GND
               to discharge any residual voltage (so that a no-touch condition
               can be properly detected). */
            GPIO_CMSIS(TS_XP_BASE)->DATA |= TS_XP_PIN;
            GPIO_CMSIS(TS_XN_BASE)->DATA &= ~TS_XN_PIN;
            GPIO_CMSIS(TS_YP_BASE)->DATA &= ~TS_YP_PIN;
            GPIO_CMSIS(TS_YN_BASE)->DATA &= ~TS_YN_PIN;

            /* Configure the sample sequence to capture the Y axis value. */
            ADC_CMSIS(ADC0_BASE)->SSMUX3 = TS_YP_ADC;

            /* If this is the valid Y sample state, then there is a new X/Y
               sample pair.  In that case, run the touch screen debouncer. */
            if(touchState == TS_STATE_READ_Y) {
                TouchScreenDebouncer();
            }

            /* The next sample will be an invalid X axis sample. */
            touchState = TS_STATE_SKIP_X;

            /* This state has been handled. */
            break;
        }
    }
}

/*
 *  ======== TouchScreenInit ========
 *  Initializes the touch screen driver.
 */
void
TouchScreenInit(uint32_t sysClock)
{
    /* Set the initial state of the touch screen driver's state machine. */
    touchState = TS_STATE_INIT;

    /* There is no touch screen handler initially. */
    touchHandler = 0;

    /* Enable the peripherals used by the touch screen interface. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    /* PE6/PP6/PP7/PE7 are used for the touch screen. Unlock pin E7 */
    GPIO_CMSIS(GPIO_PORTE_BASE)->LOCK = GPIO_LOCK_KEY;
    GPIO_CMSIS(GPIO_PORTE_BASE)->CR = 0xff;
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_7);
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_7);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_6);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_6, 0);
    GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_6);
    GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_6, 0);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_7, 0);
    GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_7, 0);

    /* Configure the ADC sample sequence used to read the touch screen reading. */
    ADCHardwareOversampleConfigure(ADC0_BASE, 4);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0,
        TS_YP_ADC | ADC_CTL_END | ADC_CTL_IE);
    ADCSequenceEnable(ADC0_BASE, 3);

    /* Enable the ADC sample sequence interrupt. */
    ADCIntEnable(ADC0_BASE, 3);
    IntEnable(INT_ADC0SS3);

    /* Configure the timer to trigger the sampling of the touch screen
       every 2.5 milliseconds. */
    if(((TIMER_CMSIS(TIMER5_BASE)->CTL) & TIMER_CTL_TAEN) == 0) {
        TimerConfigure(TIMER5_BASE, (TIMER_CFG_SPLIT_PAIR |
            TIMER_CFG_A_PWM | TIMER_CFG_B_PERIODIC));
    }
    TimerPrescaleSet(TIMER5_BASE, TIMER_B, 255);
    TimerLoadSet(TIMER5_BASE, TIMER_B, ((sysClock / 256) / 400) - 1);
    TimerControlTrigger(TIMER5_BASE, TIMER_B, true);

    /* Enable the timer.  At this point, the touch screen state machine will
       sample and run every 2.5 ms. */
    TimerEnable(TIMER5_BASE, TIMER_B);
}

/*
 *  ======== TouchScreenCallbackSet ========
 *  Sets the callback function for touch screen events.
 */
void
TouchScreenCallbackSet(int32_t (*callbackFxn)(uint32_t message,
        int32_t x, int32_t y))
{
    /* Save the pointer to the callback function. */
    touchHandler = callbackFxn;
}
