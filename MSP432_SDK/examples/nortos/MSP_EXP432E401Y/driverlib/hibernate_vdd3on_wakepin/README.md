# MSP432E4 example code for Hibernate VDD3ON wake-up with Wake Pin.

The example puts the device in hibernate VDD3ON mode with wake up from the dedicated WAKE_N pin.
 The device in active state switched the LED D2 ON. To put the device in hibernate the user must
 press the USR_SW1 which turns the LED D2 OFF and puts the device into hibernate state. To wakeup
 the device from hibernate the User must press the WAKE_SW. When the wake from Hibernate is
 detected the LED D1 is switched ON. The status can be cleared by pressing USR_SW2.
