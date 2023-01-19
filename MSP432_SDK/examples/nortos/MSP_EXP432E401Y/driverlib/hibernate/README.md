# Hibernate Example

An example to demonstrate the use of the Hibernation module.  The user
can put the microcontroller in hibernation by typing 'hib' in the terminal
and pressing ENTER or by pressing USR_SW1 on the board.  The
microcontroller will then wake on its own after 5 seconds, or immediately
if the user presses the RESET button.  The External WAKE button, external
WAKE pins, and GPIO (PK6) wake sources can also be used to wake
immediately from hibernation.  The following wiring enables the use of
these pins as wake sources.
    WAKE on breadboard connection header (X11-95) to GND
    PK6 on BoosterPack 2 (X7-17) to GND
    PK6 on breadboard connection header (X11-63) to GND

The program keeps a count of the number of times it has entered
hibernation.  The value of the counter is stored in the battery-backed
memory of the Hibernation module so that it can be retrieved when the
microcontroller wakes.  The program displays the wall time and date by
making use of the calendar function of the Hibernate module.  User can
modify the date and time if so desired.
