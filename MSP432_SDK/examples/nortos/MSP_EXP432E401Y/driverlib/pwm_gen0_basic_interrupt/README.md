# MSP432E4 PWM with basic interrupt

This project creates two PWMs from PWM Generator 3.  Each PWM period is 128K clock cycles(64K up and 64K down);
 ~250Hz. PWMA has a 25% positive duty cycle while PWMB has 75%. The PWM interrupt sources for Generator 0 are
 the PWMA comparator value in both the up and down directions and the Load and Zero count values.