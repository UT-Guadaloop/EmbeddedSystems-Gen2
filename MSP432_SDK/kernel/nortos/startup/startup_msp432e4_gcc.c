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

#include <stdint.h>

#define __MSP432E411Y__
#include <ti/devices/msp432e4/inc/msp432.h>

#include <ti/devices/msp432e4/driverlib/fpu.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
void resetISR(void);
static void nmiISR(void);
static void faultISR(void);
static void defaultHandler(void);
static void busFaultHandler(void);

//*****************************************************************************
//
// External declaration for the reset handler that is to be called when the
// processor is started
//
//*****************************************************************************
extern void _c_int00(void);
extern void ClockP_sysTickHandler(void);

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern int main(void);

//*****************************************************************************
//
// linker variable that marks the top of stack.
//
//*****************************************************************************
extern unsigned long _stack_end;

//*****************************************************************************
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000 or at the start of
// the program if located at a start address other than 0.
//
//*****************************************************************************
void (* const interruptVectors[])(void) __attribute__ ((section (".intvecs"))) =
{
    (void (*)(void))((uint32_t)&_stack_end),
                                         // The initial stack pointer
    resetISR,                            // The reset handler
    nmiISR,                              // The NMI handler
    faultISR,                            // The hard fault handler
    defaultHandler,                      // The MPU fault handler
    busFaultHandler,                     // The bus fault handler
    defaultHandler,                      // The usage fault handler
    0,                                   // Reserved
    0,                                   // Reserved
    0,                                   // Reserved
    0,                                   // Reserved
    defaultHandler,                      // SVCall handler
    defaultHandler,                      // Debug monitor handler
    0,                                   // Reserved
    defaultHandler,                      // The PendSV handler
    ClockP_sysTickHandler,               // The SysTick handler
    defaultHandler,                      // GPIO Port A
    defaultHandler,                      // GPIO Port B
    defaultHandler,                      // GPIO Port C
    defaultHandler,                      // GPIO Port D
    defaultHandler,                      // GPIO Port E
    defaultHandler,                      // UART0 Rx and Tx
    defaultHandler,                      // UART1 Rx and Tx
    defaultHandler,                      // SSI0 Rx and Tx
    defaultHandler,                      // I2C0 Master and Slave
    defaultHandler,                      // PWM Fault
    defaultHandler,                      // PWM Generator 0
    defaultHandler,                      // PWM Generator 1
    defaultHandler,                      // PWM Generator 2
    defaultHandler,                      // Quadrature Encoder 0
    defaultHandler,                      // ADC Sequence 0
    defaultHandler,                      // ADC Sequence 1
    defaultHandler,                      // ADC Sequence 2
    defaultHandler,                      // ADC Sequence 3
    defaultHandler,                      // Watchdog timer
    defaultHandler,                      // Timer 0 subtimer A
    defaultHandler,                      // Timer 0 subtimer B
    defaultHandler,                      // Timer 1 subtimer A
    defaultHandler,                      // Timer 1 subtimer B
    defaultHandler,                      // Timer 2 subtimer A
    defaultHandler,                      // Timer 2 subtimer B
    defaultHandler,                      // Analog Comparator 0
    defaultHandler,                      // Analog Comparator 1
    defaultHandler,                      // Analog Comparator 2
    defaultHandler,                      // System Control (PLL, OSC, BO)
    defaultHandler,                      // FLASH Control
    defaultHandler,                      // GPIO Port F
    defaultHandler,                      // GPIO Port G
    defaultHandler,                      // GPIO Port H
    defaultHandler,                      // UART2 Rx and Tx
    defaultHandler,                      // SSI1 Rx and Tx
    defaultHandler,                      // Timer 3 subtimer A
    defaultHandler,                      // Timer 3 subtimer B
    defaultHandler,                      // I2C1 Master and Slave
    defaultHandler,                      // CAN0
    defaultHandler,                      // CAN1
    defaultHandler,                      // Ethernet
    defaultHandler,                      // Hibernate
    defaultHandler,                      // USB0
    defaultHandler,                      // PWM Generator 3
    defaultHandler,                      // uDMA Software Transfer
    defaultHandler,                      // uDMA Error
    defaultHandler,                      // ADC1 Sequence 0
    defaultHandler,                      // ADC1 Sequence 1
    defaultHandler,                      // ADC1 Sequence 2
    defaultHandler,                      // ADC1 Sequence 3
    defaultHandler,                      // External Bus Interface 0
    defaultHandler,                      // GPIO Port J
    defaultHandler,                      // GPIO Port K
    defaultHandler,                      // GPIO Port L
    defaultHandler,                      // SSI2 Rx and Tx
    defaultHandler,                      // SSI3 Rx and Tx
    defaultHandler,                      // UART3 Rx and Tx
    defaultHandler,                      // UART4 Rx and Tx
    defaultHandler,                      // UART5 Rx and Tx
    defaultHandler,                      // UART6 Rx and Tx
    defaultHandler,                      // UART7 Rx and Tx
    defaultHandler,                      // I2C2 Master and Slave
    defaultHandler,                      // I2C3 Master and Slave
    defaultHandler,                      // Timer 4 subtimer A
    defaultHandler,                      // Timer 4 subtimer B
    defaultHandler,                      // Timer 5 subtimer A
    defaultHandler,                      // Timer 5 subtimer B
    defaultHandler,                      // FPU
    0,                                      // Reserved
    0,                                      // Reserved
    defaultHandler,                      // I2C4 Master and Slave
    defaultHandler,                      // I2C5 Master and Slave
    defaultHandler,                      // GPIO Port M
    defaultHandler,                      // GPIO Port N
    0,                                      // Reserved
    defaultHandler,                      // Tamper
    defaultHandler,                      // GPIO Port P (Summary or P0)
    defaultHandler,                      // GPIO Port P1
    defaultHandler,                      // GPIO Port P2
    defaultHandler,                      // GPIO Port P3
    defaultHandler,                      // GPIO Port P4
    defaultHandler,                      // GPIO Port P5
    defaultHandler,                      // GPIO Port P6
    defaultHandler,                      // GPIO Port P7
    defaultHandler,                      // GPIO Port Q (Summary or Q0)
    defaultHandler,                      // GPIO Port Q1
    defaultHandler,                      // GPIO Port Q2
    defaultHandler,                      // GPIO Port Q3
    defaultHandler,                      // GPIO Port Q4
    defaultHandler,                      // GPIO Port Q5
    defaultHandler,                      // GPIO Port Q6
    defaultHandler,                      // GPIO Port Q7
    defaultHandler,                      // GPIO Port R
    defaultHandler,                      // GPIO Port S
    defaultHandler,                      // SHA/MD5 0
    defaultHandler,                      // AES 0
    defaultHandler,                      // DES3DES 0
    defaultHandler,                      // LCD Controller 0
    defaultHandler,                      // Timer 6 subtimer A
    defaultHandler,                      // Timer 6 subtimer B
    defaultHandler,                      // Timer 7 subtimer A
    defaultHandler,                      // Timer 7 subtimer B
    defaultHandler,                      // I2C6 Master and Slave
    defaultHandler,                      // I2C7 Master and Slave
    defaultHandler,                      // HIM Scan Matrix Keyboard 0
    defaultHandler,                      // One Wire 0
    defaultHandler,                      // HIM PS/2 0
    defaultHandler,                      // HIM LED Sequencer 0
    defaultHandler,                      // HIM Consumer IR 0
    defaultHandler,                      // I2C8 Master and Slave
    defaultHandler,                      // I2C9 Master and Slave
    defaultHandler                       // GPIO Port T
};

//*****************************************************************************
//
// The following are arrays of pointers to constructor functions that need to
// be called during startup to initialize global objects.
//
//*****************************************************************************
extern void (*__init_array_start []) (void);
extern void (*__init_array_end []) (void);

//*****************************************************************************
//
// The following global variable is required for C++ support.
//
//*****************************************************************************
void * __dso_handle = (void *) &__dso_handle;

//*****************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.  Only the absolutely necessary set is performed,
// after which the application supplied entry() routine is called.  Any fancy
// actions (such as making decisions based on the reset cause register, and
// resetting the bits in that register) are left solely in the hands of the
// application.
//
//*****************************************************************************
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __data_load__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;

void resetISR(void)
{
    uint32_t * bs;
    uint32_t * be;
    uint32_t * dl;
    uint32_t * ds;
    uint32_t * de;
    uint32_t count;
    uint32_t i;

    IntMasterDisable();

    /* Enable FPU */
    FPUEnable();

    /* Set the frequency to 120MHz */
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                        SYSCTL_OSC_MAIN |
                        SYSCTL_USE_PLL |
                        SYSCTL_CFG_VCO_480),
                        120000000);

    /* initiailize .bss to zero */
    bs = & __bss_start__;
    be = & __bss_end__;
    while (bs < be) {
        *bs = 0;
        bs++;
    }

    /* relocate the .data section */
    dl = & __data_load__;
    ds = & __data_start__;
    de = & __data_end__;
    if (dl != ds) {
        while (ds < de) {
            *ds = *dl;
            dl++;
            ds++;
        }
    }

    /* Run any constructors */
    count = (uint32_t)(__init_array_end - __init_array_start);
    for (i = 0; i < count; i++) {
        __init_array_start[i]();
    }

    /* Call the application's entry point. */
    main();
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
static void
nmiISR(void)
{
    /* Enter an infinite loop. */
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
faultISR(void)
{
    /* Enter an infinite loop. */
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************

static void
busFaultHandler(void)
{
    /* Enter an infinite loop. */
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
defaultHandler(void)
{
    /* Enter an infinite loop. */
    while(1)
    {
    }
}

//*****************************************************************************
//
// This function is called by __libc_fini_array which gets called when exit()
// is called. In order to support exit(), an empty _fini() stub function is
// required.
//
//*****************************************************************************
void _fini(void)
{
    /* Function body left empty intentionally */
}
