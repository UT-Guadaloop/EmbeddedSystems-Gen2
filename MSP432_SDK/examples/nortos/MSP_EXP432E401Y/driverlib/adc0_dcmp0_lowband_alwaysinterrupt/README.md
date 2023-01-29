# MSP432E4 Example project for ADC use for Digital Comparator

In this application example the ADC0 is configured for a single sequencer sampling
 a single channel in single ended mode. The sequencer is triggered using a Timer
 and the data from the channel is sent to the Digital Comparator block. The 
 Digital Comparator is configured to generate an interrupt continuously when the
 data is below the programmed low band. When the interrupt is triggered the LED D2
 is switched ON. The main application loop switches the LED off.