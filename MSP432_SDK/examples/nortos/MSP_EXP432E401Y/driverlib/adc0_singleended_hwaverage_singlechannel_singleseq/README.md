# MSP432E4 Example project for ADC with single channel and single sequencer with HW averaging.

In this application example the ADC0 is configured for a single sequencer sampling a single
 channel in single ended mode with averaging set for 32x. Once the channel is sampled 32 
 times, the ADC controller performs the hardware averaging and then puts the data in the 
 ADC FIFO. The data is then displayed on the serial console.
