# MSP432E4 Example project for ADC use for Digital Comparator

In this application example the ADC0 is configured for a single sequencer 
 sampling a single channel in single ended mode. The sequencer is triggered 
 using a Timer and the data from the channel is sent to the Digital 
 Comparator block. The Digital Comparator is configured to generate an 
 interrupt continuously when the data is below the programmed low band till
 it does not cross the programmed high threshold. When the interrupt is 
 generated the LED D2 is lit. Only the Switch SW2 press clears the LED if the
 signal has crossed above the programmed high band.