# MSP432E4 Example project for ADC use for Digital Comparator

In this application example the ADC1 is configured for a single sequencer
 sampling a single channel in single ended mode. The sequencer is triggered
 using a Timer and the data from the channel is sent to the Digital 
 Comparator block. The Digital Comparator is configured to generate an 
 interrupt continuously when the data is above the programmed high band 
 till it does not cross the programmed low threshold. When the interrupt is
 generated the LED D2 is lit. Only the Switch SW2 press clears the LED if
 the signal has crossed above the programmed high band.