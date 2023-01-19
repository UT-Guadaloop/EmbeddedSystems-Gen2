# MSP432E4 Example Project to demonstrate Bit Banded output.

In this example the SysTick Timer is configured to generate an interrupt once every
 1 second and on every interrupt the LED D2 Port pin is written. The data to be 
 written is switched from 00 -> 01 -> 10 -> 11 -> 00.