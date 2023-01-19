# MSP432E4 Example project for ADC use for Digital Comparator

In this application example the ADC0 is configured for a single sequencer
 sampling a single channel in single ended mode. The sequencer is triggered
 using a Timer and the data from the channel is sent to the Digital 
 Comparator block. The Digital Comparator is configured to generate an 
 interrupt continuously when the data enters the mid-band either from the
 programmed low band or high band. When the interrupt is generated the LED
 D2 is lit. The LED auto clears once the condition for mid band is resolved.
