# MSP432E4 Example project for ADC with multiple channel and single sequencer

In this application example the ADC0 is configured for a single sequencer sampling
 4 channels in single ended mode. Each channel is configured for a different sample
 and hold duration. AIN0 is configured for 32 clock cycle sample and hold, AIN1 is
 configured for 16 cycle sample and hold, AIN2 is configured for 64 cycle sample 
 and hold & AIN3 is configured for 128 clock cycle sample and hold. The data is then
 displayed on the serial console.