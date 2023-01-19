# MSP432E4 Example project for Configuring a timer in 16-bit PWM mode with DMA Request

In this example, the timer is configured to generate a PWM output with a frequency of
 2 KHz and 66% duty cycle. The DMA Request is generated on every rising edge of the PWM
 signal. The DMA request is used to copy data from one SRAM buffer to another in AUTO Mode.