# Tamper Example

An example to demonstrate the use of tamper function in
Hibernate module. The user can ground any of these four GPIO pins
(PM4, PM5, PM6, PM7 on J28 and J30 headers on the development kit) to
manually trigger tamper events(s). The on-board LEDs reflect which pin
has triggered a tamper event. The user can put the system in hibernation
by pressing the USR_SW1 button. The system should wake when the user
either press RESET button, or ground any of the four pins to trigger
tamper event(s).

WARNING: XOSC failure is implemented in this example code, care must be
taken to ensure that the XOSCn pin(Y3) is properly grounded in order to
safely generate the external oscillator failure without damaging the
external oscillator. XOSCFAIL can be triggered as a tamper event,
as well as wakeup event from hibernation.
