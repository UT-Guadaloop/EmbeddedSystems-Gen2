# Watchdog

This example application demonstrates the use of the watchdog as a simple
heartbeat for the system.  If the watchdog is not periodically fed, it will
reset the system.  Each time the watchdog is fed, the LED is inverted so
that it is easy to see that it is being fed, which occurs once every
second.  To stop the watchdog being fed and, hence, cause a system reset,
press the SW1 button.
