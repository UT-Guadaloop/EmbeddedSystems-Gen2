# MSP432E4 example code for Hibernate VDD3ON wake-up with Tamper Detection.

The example puts the device in hibernate VDD3ON mode with wake up due to Tamper Detection.
 The Tamper pins are configured to detect a low level tamper event and Crystal failure is
 also enabled. The devcie enters hibernate when user presses the switch SW1. When a Tamper
 event occurs the NMI is called.
