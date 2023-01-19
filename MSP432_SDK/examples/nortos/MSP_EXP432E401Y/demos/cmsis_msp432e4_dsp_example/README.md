# MSP432E4 example code for CMSIS DSP Library.

The ADC is configured to sample AIN0 channel using a timer trigger at 100 KHz.
 The DMA is configured to transfer the data from the ADC to SRAM buffer using
 Ping-Pong mechanism. When the data buffer is copied the ADC gives a DMA Done
 interrupt to which the CPU first re-initializes the DMA and the performs 
 sample-averaging for DC value, RMS calculation and FFT of the data and 
 displays on the serial console the DC average, RMS, maximum FFT energy and 
 FFT frequency bin at which maximum energy is detected.