# MSP432E4 Example project for Configuring a multiple timer in 16-bit PWM mode and sync.

In this example, the first timer is configured to generate a PWM output with a frequency of
 2 KHz and 66% duty cycle and another timer with PWM output with a frequency of 2.001 kHz 
 and 33% duty cycle. The output for the two timers are out of sync. When the user presses 
 the switch SW2 the application synchronizes the output causing the PWM output to be aligned.
 In between switch presses the two outputs shall drift and the drift can be seen on a scope
 or a logic analyzer.