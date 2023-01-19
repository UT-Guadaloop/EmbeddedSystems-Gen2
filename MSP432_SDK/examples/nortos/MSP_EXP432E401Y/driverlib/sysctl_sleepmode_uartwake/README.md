# MSP432E4 Example for Sleep Mode entry and wakeup using UART

The application code puts the device in sleep mode with clock enabled only for the UART
 in sleep mode and memory in standby. During Active state (RUN Mode), the LED D1 is
 switched ON. When there is no activity on the terminal application the device is put
 into sleep mode and LED D1 is switched OFF. When the user types a character in the
 terminal the device is woken up and the LED D1 switches ON.