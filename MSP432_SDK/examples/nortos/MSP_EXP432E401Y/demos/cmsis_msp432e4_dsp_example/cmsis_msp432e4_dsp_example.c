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
 * MSP432E4 example code for CMSIS DSP Library.
 *
 * Description: The ADC is configured to sample AIN0 channel using a timer
 * trigger at 100 KHz. The DMA is configured to transfer the data from the ADC
 * to SRAM buffer using Ping-Pong mechanism. When the data buffer is copied the
 * ADC gives a DMA Done interrupt to which the CPU first re-initializes the DMA
 * and the performs sample-averaging for DC value, RMS calculation and FFT of
 * the data and displays on the serial console the DC average, RMS, maximum
 * FFT energy and FFT frequency bin at which maximum energy is detected.
 *
 * CMSIS DSP Function Used:
 * 1. arm_sqrt_f32
 * 2. arm_cfft_q15
 * 3. arm_cmplx_mag_q15
 * 4. arm_max_q15
 *
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST            PE3|<--AIN0
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 * Author: Amit Ashara
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

#include "arm_math.h"
#include "arm_const_structs.h"

/* Define for Samples to be captured and Sampling Frequency */
#define NUM_SAMPLES 1024
#define SAMP_FREQ   100000

/* DMA Buffer declaration and buffer complet flag */
static uint32_t dstBufferA[NUM_SAMPLES];
static uint32_t dstBufferB[NUM_SAMPLES];
volatile bool setBufAReady = false;
volatile bool setBufBReady = false;

/* Global variables and defines for FFT */
#define IFFTFLAG   0
#define BITREVERSE 1
volatile int16_t fftOutput[NUM_SAMPLES*2];

/* Global variables for RMS and DC calculation */
volatile float32_t rmsBuff;
volatile float32_t dcBuff;
float32_t rmsCalculation;

/* The control table used by the uDMA controller.  This table must be aligned
 * to a 1024 byte boundary. */
#if defined(__ICCARM__)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

void ADC0SS2_IRQHandler(void)
{
    uint32_t getIntStatus;
    uint32_t getDMAStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* Clear the ADC interrupt flag. */
    MAP_ADCIntClearEx(ADC0_BASE, getIntStatus);

    /* Read the primary and alternate control structures to find out which
     * of the structure has completed and generated the done interrupt. Then
     * re-initialize the appropriate structure */
    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH16_ADC0_2 |
                                          UDMA_PRI_SELECT);

    /* Check if the primary or alternate channel has completed. On completion
     * re-initalize the channel control structure. If the Primary channel has
     * completed then set Buffer-A ready flag so that the main application
     * may perform the DSP computation. Similarly if the Alternate channel
     * has completed then set Buffer-B ready flag so that the main application
     * may perform the DSP computation. */
    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
                                   sizeof(dstBufferA)/4);
        setBufAReady = true;
    }

    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH16_ADC0_2 |
                                          UDMA_ALT_SELECT);

    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&ADC0->SSFIFO2, (void *)&dstBufferB,
                                   sizeof(dstBufferB)/4);
        setBufBReady = true;
    }
}

void ConfigureUART(uint32_t systemClock)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200 bps 8-N-1 format */
    UARTStdioConfig(0, 115200, systemClock);
}

int main(void)
{
    uint32_t systemClock;
    uint32_t ii;
    uint32_t setFFTmaxValue;
    uint32_t setFFTmaxFreqIndex;
    int_fast32_t i32IPart[3];
    int_fast32_t i32FPart[3];

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                          SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL |
                                          SYSCTL_CFG_VCO_480), 120000000);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Display the setup on the console. */
    UARTprintf("\033[2J\033[H");
    UARTprintf("\rCMSIS DSP Demo...\n\n");
    UARTprintf("\033[2GDC Average \033[31G\n");
    UARTprintf("\033[2GRMS \033[31G\n");
    UARTprintf("\033[2GFFT Amplitude \033[31G\n");
    UARTprintf("\033[2GFFT Frequency \033[31G\n");


    /* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }

    /* Configure PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
     * conversion and interrupt generation is set for AIN0 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 2 with a timer signal trigger.  Sequencer 2
     * will do a single sample when the timer generates a trigger on timeout*/
    MAP_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_TIMER, 2);

    /* Clear the interrupt status flag before enabling. This is done to make
     * sure the interrupt flag is cleared before we sample. */
    MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS2);

    /* Enable the DMA request from ADC0 Sequencer 2 */
    MAP_ADCSequenceDMAEnable(ADC0_BASE, 2);

    /* Since sample sequence 2 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 2);

    /* Enable the Interrupt generation from the ADC-0 Sequencer */
    MAP_IntEnable(INT_ADC0SS2);

    /* Enable the DMA and Configure Channel for TIMER0A for Ping Pong mode of
     * transfer */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
    {
    }

    MAP_uDMAEnable();

    /* Point at the control table to use for channel control structures. */
    MAP_uDMAControlBaseSet(pui8ControlTable);

    /* Map the ADC0 Sequencer 2 DMA channel */
    MAP_uDMAChannelAssign(UDMA_CH16_ADC0_2);

    /* Put the attributes in a known state for the uDMA ADC0 Sequencer 2
     * channel. These should already be disabled by default. */
    MAP_uDMAChannelAttributeDisable(UDMA_CH16_ADC0_2,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    /* Configure the control parameters for the primary control structure for
     * the ADC0 Sequencer 2 channel. The primary control structure is used for
     * copying the data from ADC0 Sequencer 2 FIFO to dstBufferA. The transfer
     * data size is 32 bits and the source address is not incremented while
     * the destination address is incremented at 32-bit boundary.
     */
    MAP_uDMAChannelControlSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE |
                              UDMA_DST_INC_32 | UDMA_ARB_1);

    /* Set up the transfer parameters for the ADC0 Sequencer 2 primary control
     * structure. The mode is Basic mode so it will run to completion. */
    MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
                               sizeof(dstBufferA)/4);

    /* Configure the control parameters for the alternate control structure for
     * the ADC0 Sequencer 2 channel. The alternate control structure is used for
     * copying the data from ADC0 Sequencer 2 FIFO to dstBufferB. The transfer
     * data size is 32 bits and the source address is not incremented while
     * the destination address is incremented at 32-bit boundary.
     */
    MAP_uDMAChannelControlSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_NONE |
                              UDMA_DST_INC_32 | UDMA_ARB_1);

    /* Set up the transfer parameters for the ADC0 Sequencer 2 alternate
     * control structure. The mode is Basic mode so it will run to
     * completion */
    MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *)&ADC0->SSFIFO2, (void *)&dstBufferB,
                               sizeof(dstBufferB)/4);

    /* The uDMA ADC0 Sequencer 2 channel is primed to start a transfer. As
     * soon as the channel is enabled and the Timer will issue an ADC trigger,
     * the ADC will perform the conversion and send a DMA Request. The data
     * transfers will begin. */
    MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);

    /* Enable Timer-0 clock and configure the timer in periodic mode with
     * a frequency of 1 KHz. Enable the ADC trigger generation from the
     * timer-0. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (systemClock/SAMP_FREQ));
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

    /* While loop to process the data */
    while(1)
    {
        /* Wait for Primary channel to complete and then clear the flag and
         * initialize the variables */
        while(!setBufAReady);
        setBufAReady = false;
        dcBuff       = 0.0f;
        rmsBuff      = 0.0f;

        /* First convert the sampled data to floating point format as the RMS
         * and DC average is being computed */
        for(ii=0;ii<NUM_SAMPLES;ii++)
        {
            rmsBuff     += ((float32_t)(dstBufferA[ii]*dstBufferA[ii])*(float32_t)(33*33))/(float32_t)(40960*40960);
            dcBuff      += (float32_t)(dstBufferA[ii]*33)/(float32_t)40960;
        }

        /* Now calculate the final DC Average and RMS */
        dcBuff = dcBuff/((float32_t)NUM_SAMPLES);
        rmsBuff = rmsBuff/((float32_t)NUM_SAMPLES);
        arm_sqrt_f32(rmsBuff,&rmsCalculation);

        /* Compute the 1024 point FFT on the sampled data and then find the
         * FFT point for maximum energy and the energy value */
        arm_cfft_q15(&arm_cfft_sR_q15_len1024, (q15_t *)dstBufferA, IFFTFLAG,
                     BITREVERSE);
        arm_cmplx_mag_q15((q15_t *)dstBufferA, (q15_t *)fftOutput,
                          NUM_SAMPLES);
        arm_max_q15((q15_t *)fftOutput, NUM_SAMPLES, (q15_t *)&setFFTmaxValue,
                    &setFFTmaxFreqIndex);

        /* Convert the floating point values to integer and fractional parts
         * for display on the serial console */
        i32IPart[0] = (int32_t) rmsCalculation;
        i32FPart[0] = (int32_t) (rmsCalculation*1000.0f);
        i32FPart[0] = (int32_t)(i32FPart[0] - i32IPart[0]*1000.0);
        i32IPart[1] = (int32_t) dcBuff;
        i32FPart[1] = (int32_t) (dcBuff*1000.0f);
        i32FPart[1] = (int32_t)(i32FPart[1] - i32IPart[1]*1000.0);
        i32IPart[2] = (int32_t) ((setFFTmaxFreqIndex*SAMP_FREQ)/NUM_SAMPLES);
        i32FPart[2] = (int32_t) (((setFFTmaxFreqIndex*SAMP_FREQ)/NUM_SAMPLES)*1000);
        i32FPart[2] = i32FPart[2] - i32IPart[2]*1000;


        /* Print the DC Average, RMS, Maximum FFT Amplitude and the FFT
         * frequency at which Max FFT Amplitude is detected */
        UARTprintf("\033[3;17H%3d.%03d Volts", i32IPart[0], i32FPart[0]);
        UARTprintf("\033[4;17H%3d.%03d Volts", i32IPart[1], i32FPart[1]);
        UARTprintf("\033[5;19H%03d", setFFTmaxValue);
        UARTprintf("\033[6;17H%3d.%03d Hz", i32IPart[2], i32FPart[2]);

        /* Wait for Alternate channel to complete and then clear the flag and
         * initialize the variables */
        while(!setBufBReady);
        setBufBReady = false;
        dcBuff       = 0.0f;
        rmsBuff      = 0.0f;

        /* First convert the sampled data to floating point format as the RMS
         * and DC average is being computed */
        for(ii=0;ii<NUM_SAMPLES;ii++)
        {
            rmsBuff     += ((float32_t)(dstBufferB[ii]*dstBufferB[ii])*(float32_t)(33*33))/(float32_t)(40960*40960);
            dcBuff      += (float32_t)(dstBufferB[ii]*33)/(float32_t)40960;
        }

        /* Now calculate the final DC Average and RMS */
        dcBuff = dcBuff/((float32_t)NUM_SAMPLES);
        rmsBuff = rmsBuff/((float32_t)NUM_SAMPLES);
        arm_sqrt_f32(rmsBuff,&rmsCalculation);

        /* Compute the 1024 point FFT on the sampled data and then find the
         * FFT point for maximum energy and the energy value */
        arm_cfft_q15(&arm_cfft_sR_q15_len1024, (q15_t *)dstBufferB, IFFTFLAG,
                     BITREVERSE);
        arm_cmplx_mag_q15((q15_t *)dstBufferB, (q15_t *)fftOutput,
                          NUM_SAMPLES);
        arm_max_q15((q15_t *)fftOutput, NUM_SAMPLES, (q15_t *)&setFFTmaxValue,
                    &setFFTmaxFreqIndex);

        /* Convert the floating point values to integer and fractional parts
         * for display on the serial console */
        i32IPart[0] = (int32_t) rmsCalculation;
        i32FPart[0] = (int32_t) (rmsCalculation*1000.0f);
        i32FPart[0] = (int32_t)(i32FPart[0] - i32IPart[0]*1000.0);
        i32IPart[1] = (int32_t) dcBuff;
        i32FPart[1] = (int32_t) (dcBuff*1000.0f);
        i32FPart[1] = (int32_t)(i32FPart[1] - i32IPart[1]*1000.0);
        i32IPart[2] = (int32_t) ((setFFTmaxFreqIndex*SAMP_FREQ)/NUM_SAMPLES);
        i32FPart[2] = (int32_t) (((setFFTmaxFreqIndex*SAMP_FREQ)/NUM_SAMPLES)*1000);
        i32FPart[2] = i32FPart[2] - i32IPart[2]*1000;

        /* Print the DC Average, RMS, Maximum FFT Amplitude and the FFT
         * frequency at which Max FFT Amplitude is detected */
        UARTprintf("\033[3;17H%3d.%03d", i32IPart[0], i32FPart[0]);
        UARTprintf("\033[4;17H%3d.%03d", i32IPart[1], i32FPart[1]);
        UARTprintf("\033[5;19H%03d", setFFTmaxValue);
        UARTprintf("\033[6;17H%3d.%03d Hz", i32IPart[2], i32FPart[2]);
    }
}
