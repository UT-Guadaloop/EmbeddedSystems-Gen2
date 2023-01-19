# GPIO JTAG Recovery

This example demonstrates changing the JTAG pins into GPIOs, a with a
mechanism to revert them to JTAG pins.  When first run, the pins remain in
JTAG mode.  Pressing the USR_SW1 button will toggle the pins between JTAG
mode and GPIO mode.  Because there is no debouncing of the push button
(either in hardware or software), a button press will occasionally result
in more than one mode change.

In this example, four pins (PC0, PC1, PC2, and PC3) are switched.

UART0, connected to the ICDI virtual COM port and running at 115,200,
8-N-1, is used to display messages from this application.
