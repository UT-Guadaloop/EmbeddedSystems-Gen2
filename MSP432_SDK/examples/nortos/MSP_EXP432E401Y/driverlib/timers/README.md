# Timer

This example application demonstrates the use of the timers to generate
periodic interrupts.  One timer is set up to interrupt once per second and
the other to interrupt twice per second; each interrupt handler will toggle
its own indicator throught the UART.

UART0, connected to the Virtual Serial Port and running at 115,200, 8-N-1,
is used to display messages from this application.
