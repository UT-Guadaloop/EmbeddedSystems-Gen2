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
 * MSP432 example code for Hibernate with Tamper Detection
 *
 * Description: An example to demonstrate the use of tamper function in
 * Hibernate module. The user can ground any of these four GPIO pins
 * (PM4, PM5, PM6, PM7 on J28 and J30 headers on the development kit) to
 * manually trigger tamper events(s). The on-board LEDs reflect which pin
 * has triggered a tamper event. The user can put the system in hibernation
 * by pressing the USR_SW1 button. The system should wake when the user
 * either press RESET button, or ground any of the four pins to trigger
 * tamper event(s).
 *
 * WARNING: XOSC failure is implemented in this example code, care must be
 * taken to ensure that the XOSCn pin(Y3) is properly grounded in order to
 * safely generate the external oscillator failure without damaging the
 * external oscillator. XOSCFAIL can be triggered as a tamper event,
 * as well as wakeup event from hibernation.
 *
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "pinout.h"
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "ti/devices/msp432e4/inc/msp.h"

//*****************************************************************************
//
// Global variables used for Tamper events.
//
//*****************************************************************************
static volatile uint32_t g_ui32TamperEventFlag = 0;
static volatile uint32_t g_ui32TamperXOSCFailEvent = 0;

/* Flag that informs that the user has requested hibernation */
static volatile bool g_bHibernate = false;

//*****************************************************************************
//
// Defines for Hibernate memory value. It is used to determine if a wakeup is
// due to a tamper event.
//
//*****************************************************************************
#define HIBERNATE_TAMPER_DATA0  0xdeadbeef

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void GPIOJ_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the GPIO and clear the status */
    getIntStatus = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if((getIntStatus & GPIO_PIN_0) == GPIO_PIN_0)
    {
        MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

        /*
         * The button has been touched.  So indicate that the
         * user wants the device to enter hibernation.
         */
        g_bHibernate = true;

        /* Turn off LED */
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);
    }

    /* Safety button in case system gets stuck in hibernation and needs to be reset*/
    if((getIntStatus & GPIO_PIN_1) == GPIO_PIN_1) {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_PIN_0 | GPIO_PIN_4);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1);
        while(1);
    }
}


void NMI_Handler(void)
{
    uint32_t ui32TamperStatus;
    uint8_t  ui8Idx;
    uint32_t ui32EventLog[4];
    uint32_t ui32RTCLog[4];

    /* Get the tamper event. */
    ui32TamperStatus = MAP_HibernateTamperStatusGet();


    /* Log the tamper event data before clearing tamper events. */
    for(ui8Idx = 0; ui8Idx< 4; ui8Idx++)
    {
        if(MAP_HibernateTamperEventsGet(ui8Idx,
                                    &ui32RTCLog[ui8Idx],
                                    &ui32EventLog[ui8Idx])) {

            /* If the timestamp entry has zero, ignore the tamper
             * log entry, otherwise, save the event. */
            if(ui32RTCLog[ui8Idx]) {
                g_ui32TamperEventFlag |= ui32EventLog[ui8Idx];

            }
            else {
                /* Not valid event in this log entry. Done checking the logs. */
                break;
            }
        }
        else {
            /* No event in this log entry. Done checking the logs. */
            break;
        }
    }

    /* Process external oscillator failed event. */
    if(ui32TamperStatus & HIBERNATE_TAMPER_STATUS_EXT_OSC_FAILED) {
        g_ui32TamperXOSCFailEvent++;
    }

    MAP_HibernateTamperEventsClear();



    /* Update GPIO indicator lights and status list box */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);

    if (g_ui32TamperEventFlag & HIBERNATE_TAMPER_EVENT_0){
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);
    }

    if (g_ui32TamperEventFlag & HIBERNATE_TAMPER_EVENT_1) {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);
    }

    if (g_ui32TamperEventFlag & HIBERNATE_TAMPER_EVENT_2) {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);
    }

    if (g_ui32TamperEventFlag & HIBERNATE_TAMPER_EVENT_3) {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
    else {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);
    }

    if (g_ui32TamperEventFlag & HIBERNATE_TAMPER_EVENT_EXT_OSC) {
        MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_PIN_0 | GPIO_PIN_4);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1);
    }

    g_ui32TamperEventFlag = 0;

}

//*****************************************************************************
//
// Returns whether the system has come out of Hibernation due to a tamper
// event or reset event.
//
//*****************************************************************************
void
HibernateTamperWakeUp(bool *pbWakeupFromTamper, bool *pbWakeupFromReset)
{
    uint32_t ui32TempBuf[2];

    /* Clear both flags. */
    *pbWakeupFromReset  = false;
    *pbWakeupFromTamper = false;

    /* Read the status bits to see what caused the wake.  Clear the wake
     * source so that the device can be put into hibernation again. */
    ui32TempBuf[0] = MAP_HibernateIntStatus(0);
    MAP_HibernateIntClear(ui32TempBuf[0]);

    /* Check the wake was due to reset. */
    if(ui32TempBuf[0] & HIBERNATE_INT_RESET_WAKE)
    {
        *pbWakeupFromReset = true;
        return;
    }

    /* The wake was not due to reset. Check if it is due to tamper event. */

    /* Read the Hibernate module memory registers that show the state of the
     * system. */
    HibernateDataGet(ui32TempBuf, 3);

    /* Determine if system came out of hibernation due to a tamper event. */
    if(ui32TempBuf[0] == HIBERNATE_TAMPER_DATA0)
    {
        /* It is due to a tamper event.
         * Read the saved tamper event and RTC log info from the hibernate
         * memory, so the main routine can print the info on the display.
         */
        g_ui32TamperEventFlag = ui32TempBuf[1];

        *pbWakeupFromTamper = true;
        return;
    }

    return;
}

//*****************************************************************************
//
// This example demonstrates the use of the Tamper module.
//
//*****************************************************************************
int
main(void)
{

    uint32_t ui32SysClock;
    uint32_t ui32Index;
    bool bWakeFromTamper;
    bool bWakeFromReset;

    /* Configure the system clock for 120 MHz */
    ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_480), 120000000);

    /* Configure the device pins. */
    PinoutSet(false, false);

    /* Enable the LEDs and turn on LED D1 */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, 0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1, (~GPIO_PIN_0)|GPIO_PIN_1);


    /* Configure the GPIO buttons, USR_SW1 puts device in hibernation */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR |= GPIO_PIN_0;
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);

    /* For debugging, prevents device from getting stuck in hibernation */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOJ->PUR |= GPIO_PIN_1;
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_BOTH_EDGES);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_1);


    MAP_IntEnable(INT_GPIOJ);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE)))
    {
    }

    /* Check to see if the processor is waking from a hibernation. */
    HibernateTamperWakeUp(&bWakeFromTamper, &bWakeFromReset);

    if(!bWakeFromTamper && !bWakeFromReset)
    {
        /* If the system didn't wake from hibernation,
         Initialize the Hibernate module and enable the RTC/calendar mode. */

        MAP_HibernateEnableExpClk(ui32SysClock);
        MAP_HibernateRTCEnable();
        MAP_HibernateCounterMode(HIBERNATE_COUNTER_24HR);


        /* Configure Hibernate wake sources. */
        MAP_HibernateWakeSet(HIBERNATE_WAKE_RESET);

        /* Configure the TPIO0~3 signals by enabling trigger on low,
         weak pull-up and glitch filtering. */
        for(ui32Index = 0; ui32Index < 4; ui32Index++)
        {
            MAP_HibernateTamperIOEnable(ui32Index,
                                    HIBERNATE_TAMPER_IO_TRIGGER_LOW |
                                    HIBERNATE_TAMPER_IO_WPU_ENABLED |
                                    HIBERNATE_TAMPER_IO_MATCH_SHORT);
        }

        /* Configure Hibernate module to wake up from tamper event. */
        MAP_HibernateTamperEventsConfig(HIBERNATE_TAMPER_EVENTS_HIB_WAKE);

        /* Enable Tamper Module. */
        MAP_HibernateTamperEnable();
    }
    else
    {
        if(bWakeFromTamper &&
           (g_ui32TamperEventFlag & HIBERNATE_TAMPER_EVENT_EXT_OSC))
        {

            /*
             *  XOSCFAIL was used to trigger a tamper event, set the flag
             * g_ui32TamperXOSCFailEvent so that it will clear external
             * oscillator failure after the external oscillator becomes
             * active.
             */
            g_ui32TamperXOSCFailEvent = 1;
        }
    }


    /* Enable interrupts. */
    MAP_IntMasterEnable();

    g_bHibernate = false;


    while(1)
    {
        /* Wait for the user to request hibernation. */
        if(g_bHibernate == true)
        {
            /* Clear the flag. */
            g_bHibernate = false;

            /* Read and clear any status bits that might have been set since
             last clearing them. */
            ui32Index = MAP_HibernateIntStatus(0);
            MAP_HibernateIntClear(ui32Index);

            /* Request Hibernation. */
            MAP_HibernateRequest();

            /* Wait for a while for hibernate to activate.  It should never
             get past this point. */
            MAP_SysCtlDelay(100);
        }

        /* If we have external oscillator failure, wait till the external
         * oscillator becomes active, then clear external oscillator failure. */
    if(g_ui32TamperXOSCFailEvent)
        {
            while(MAP_HibernateTamperExtOscValid() == 0);
            MAP_HibernateTamperExtOscRecover();
            g_ui32TamperXOSCFailEvent = 0;
        }
    }

}
