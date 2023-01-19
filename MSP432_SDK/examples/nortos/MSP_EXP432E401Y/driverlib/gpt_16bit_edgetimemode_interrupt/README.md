# MSP432E4 Example project for Configuring a timer in 16-bit Edge Time mode

In this example, the timer is configured in Event Time mode. The GPIO port J Pull Up
 is enabled so that a switch SW2 press causes an event to be generated. The timer
 captures the falling edge and generates an interrupt. The time stamp is captured and
 printed on the UART console.