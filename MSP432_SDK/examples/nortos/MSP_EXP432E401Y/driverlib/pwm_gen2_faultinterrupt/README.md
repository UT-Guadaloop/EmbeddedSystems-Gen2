# MSP432E4 PWM with fault interrupt Project

PWMs are generated on PG1/PG0 but gated upon fault source, PK7. The first instance of the fault the PWM is gated by the
 duration of the fault.  This fault will have a minimum duration of 32K PWM clock cycles and a maximum defined by the
 fault duration. On the second fault,  the PWM is halted until the fault is cleared by a rising edge on PJ0.