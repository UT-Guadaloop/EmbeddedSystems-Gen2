# MSP432E4 Example project for ADC use for Digital Comparator

In this application example the ADC0 is configured for a single sequencer 
 sampling a single channel in single ended mode. The sequencer is triggered
 using a Timer and the data from the channel is sent to the Digital 
 Comparator block. The Digital Comparator is configured to generate an 
 interrupt once when the data is above the programmed high band. When the
 interrupt is triggered the LED D2 is toggled.