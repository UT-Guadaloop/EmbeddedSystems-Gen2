/* --COPYRIGHT--,BSD
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
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432E4 PWM with fault interrupt Project
 *
 * Description: PWMs are generated on PG1/PG0 but gated upon fault source, PK7.
 *              The first instance of the fault the PWM is gated by the
 *              duration of the fault.  This fault will have a minimum duration
 *              of 32K PWM clock cycles and a maximum defined by the fault
 *              duration.
 *              On the second fault,  the PWM is halted until the fault is
 *              cleared by a rising edge on PJ0.
 *
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |       PG0(M0PWM4)|---> PWMA(PWM_GEN2) 75% Duty Cycle
 *            |       PG1(M0PWM5)|---> PWMA(PWM_GEN2) 25% Duty Cycle
 *            |                  |
 *            |               PJ0|<-- USR_SW1
 *            |     PK7(M0FAULT2)|<-- Fault Source 2
 *
 * Author: C. Sterzik
 *
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines to be used in the project */
#define MIN_PERIOD    0
#define LATCH         1
#define LATCHED       2

/* Global variable for system clock */
uint32_t getSystemClock;
uint8_t  getFaultMode;

/* PWM ISR, PWM0 GEN2 */
void PWM0_FAULT_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the Base0 PWM Masked interrupt status*/
    getIntStatus = MAP_PWMIntStatus(PWM0_BASE, true);

    if(getIntStatus & PWM_INT_FAULT2)
    {
        if(getFaultMode == LATCH)
        {
            /* Disable interrupt and return to main to check for cleared fault
             */
            MAP_PWMIntDisable(PWM0_BASE, PWM_INT_FAULT2);
            getFaultMode = LATCHED;
        }
        if(getFaultMode == MIN_PERIOD)
        {
            /* Remain in ISR until fault is removed */
            while(MAP_PWMIntStatus(PWM0_BASE, true) & PWM_INT_FAULT2)
            {
                MAP_PWMFaultIntClearExt(PWM0_BASE, PWM_INT_FAULT2);
                /* The PWM_INT_FAULT2 will remain cleared until the fault
                 * signal returns to the low position or the minimum fault
                 * period is expired, whichever is greater. */
            }
            MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN |
                                PWM_GEN_MODE_NO_SYNC |
                                PWM_GEN_MODE_FAULT_LATCHED |
                                PWM_GEN_MODE_FAULT_NO_MINPER |
                                PWM_GEN_MODE_FAULT_EXT
                                );
            getFaultMode = LATCH;
        }
    }
}

/* Main application code */
int main(void)
{
    /* Configure the system clock for 16 MHz internal oscillator */
    getSystemClock = MAP_SysCtlClockFreqSet((SYSCTL_OSC_INT |
                                             SYSCTL_USE_OSC), 16000000);

    /* The PWM peripheral must be enabled for use. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)));

    /* Set the PWM clock to the system clock. */
    MAP_PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_1);

    /* For this example PWM0 is used with PortK Pins 4 and 5.  The actual port
     * and pins used may be different on your part, consult the data sheet for
     * more information.  GPIO port B needs to be enabled so these pins can be
     * used. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));

    /* Configure the GPIO pin muxing to select PWM functions for these pins.
     * This step selects which alternate function is available for these pins.
     * This is necessary if your part supports GPIO pin function muxing.
     * Consult the data sheet to see which functions are allocated per pin.
     * TODO: change this to select the port/pin you are using. */
    MAP_GPIOPinConfigure(GPIO_PG0_M0PWM4);
    MAP_GPIOPinConfigure(GPIO_PG1_M0PWM5);
    MAP_GPIOPinTypePWM(GPIO_PORTG_BASE, (GPIO_PIN_0|GPIO_PIN_1));

    /* Configure the GPIO pad for PWM function on pin K7.  Consult
     * the data sheet to see which functions are allocated per pin.
     * TODO: change this to select the port/pin you are using.
     * Fault Triggers       IO
     *  M0FAULT0            PF4
     *  M0FAULT1            PK6
     *  M0FAULT2            PK7
     *  MOFAULT3            PL0 */
    MAP_GPIOPinConfigure(GPIO_PK7_M0FAULT2);
    MAP_GPIOPinTypePWM(GPIO_PORTK_BASE, GPIO_PIN_7);

    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR |= GPIO_PIN_0;

    /* Configure the PWM0 to count up/down without synchronization. */
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN |
                        PWM_GEN_MODE_NO_SYNC |
                        PWM_GEN_MODE_FAULT_UNLATCHED |
                        PWM_GEN_MODE_FAULT_MINPER |
                        PWM_GEN_MODE_FAULT_EXT
                        );

    MAP_PWMGenFaultConfigure(PWM0_BASE,
                             PWM_GEN_2,
                             32000,              // Minimum fault of 32000 PWM clocks
                             PWM_FAULT2_SENSE_HIGH);

    MAP_PWMGenFaultTriggerSet(PWM0_BASE,
                              PWM_GEN_2,
                              PWM_FAULT_GROUP_0, // Group-0 Fault
                              PWM_FAULT_FAULT2);

    MAP_PWMOutputFault(PWM0_BASE,(PWM_OUT_4_BIT | PWM_OUT_5_BIT),true);
    MAP_PWMOutputFaultLevel(PWM0_BASE,PWM_OUT_4_BIT,true);   // output high
    MAP_PWMOutputFaultLevel(PWM0_BASE,PWM_OUT_5_BIT,false);  // output low

    /* Set the PWM period to 250Hz.  To calculate the appropriate parameter
     * use the following equation: N = (1 / f) * SysClk.  Where N is the
     * function parameter, f is the desired frequency, and SysClk is the
     * system clock frequency.
     * In this case you get: (1 / 250Hz) * 16MHz = 64000 cycles.  Note that
     * the maximum period you can set is 2^16 - 1. */
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, 64000);

    /* Set PWM0 PG0 to a duty cycle of 25%.  You set the duty cycle as a
     * function of the period.  Since the period was set above, you can use the
     * PWMGenPeriodGet() function.  For this example the PWM will be high for
     * 25% of the time or 16000 clock cycles (64000 / 4). */
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,
                     MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2) / 4);

    /* Enable the dead-band generation on the PWM0 output signal.  PWM bit 4
     * (PG0), will have a duty cycle of 25% (set above) and PWM bit 5 will have
     * a duty cycle of 75%.  These signals will have a 10us gap between the
     * rising and falling edges.  This means that before PWM bit 4 goes high,
     * PWM bit 5 has been low for at LEAST 160 cycles (or 10us) and the same
     * before PWM bit 5 goes high.  The dead-band generator lets you specify
     * the width of the "dead-band" delay, in PWM clock cycles, before the PWM
     * signal goes high and after the PWM signal falls.  For this example we
     * will use 160 cycles (or 10us) on both the rising and falling edges of
     * PG0.  Reference the datasheet for more information on dead-band
     * generation. */
    MAP_PWMDeadBandEnable(PWM0_BASE, PWM_GEN_2, 160, 160);

    MAP_IntMasterEnable();

    /* Interrupts are based upon fault for GEN 2 on PWM0. */
    MAP_IntEnable(INT_PWM0_FAULT);
    MAP_PWMIntEnable(PWM0_BASE, PWM_INT_FAULT2);

    /* Enable the PWM0 Bits 4 and 5 (PG0 and PG1) output signals. */
    MAP_PWMOutputState(PWM0_BASE, (PWM_OUT_4_BIT | PWM_OUT_5_BIT), true);

    /* Enables the counter for a PWM generator block. */
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_2);

    getFaultMode = MIN_PERIOD;

    /* Loop forever while the PWM signals are generated. */
    while(1)
    {
        if(getFaultMode == LATCHED)
        {
            /* Poll PJ0 and use this signal to gate clearing of the fault
             * since it has been latched */
            while(MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0)
            {
            }

            while(MAP_PWMIntStatus(PWM0_BASE, false) & PWM_INT_FAULT2)
            {
                /* Clear fault */
                MAP_PWMFaultIntClearExt(PWM0_BASE, PWM_INT_FAULT2);
            }

            MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN |
                                PWM_GEN_MODE_NO_SYNC |
                                PWM_GEN_MODE_FAULT_UNLATCHED |
                                PWM_GEN_MODE_FAULT_MINPER |
                                PWM_GEN_MODE_FAULT_EXT
                                );

            getFaultMode = MIN_PERIOD;

            MAP_PWMIntEnable(PWM0_BASE, PWM_INT_FAULT2);
        }
    }
}
