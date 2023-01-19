# MSP432E4 Example project for Configuring multiple timers in 16-bit PWM mode and wait on trigger.

In this example, all the timers are configured to generate PWM output of 2 kHz and 66% duty cycle.
 However the timere are configured in wait on trigger mode, such that when the first timer elapses
 it starts the next timer in PWM mode. The output of each PWM can be monitored on a LA to see how
 the timers trigger the next till all the timers generate a PWM out.