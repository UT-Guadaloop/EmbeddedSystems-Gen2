# Sleep Modes

This example demonstrates the different power modes available on the
MSP432E4 devices. The user button (USR-SW1) is used to cycle through the
different power modes.
The SRAM, Flash, and LDO are all configured to a lower power setting for
the different modes.

A timer is configured to toggle an LED in an ISR in both Run and Sleep
mode.
In Deep-Sleep the PWM is used to toggle the same LED in hardware. The three
remaining LEDs are used to indicate the current power mode.

        LED key in addition to the toggling LED:
            3 LEDs on - Run Mode
            2 LEDs on - Sleep Mode
            1 LED on - Deep-Sleep Mode

UART0, connected to the Virtual Serial Port and running at 115,200, 8-N-1,
is used to display messages from this application.
