# MSP432E4 out of box example for EPI to access SDRAM memory

The EPI is configured to access an SDRAM memory at 60MHz. The
example programs the GPIOs for EPI and configures the EPI. After the
initialization is complete, basic SDRAM Writes and Reads are performed using
16-bit, 32-bit and 64-bit. SysTick Timer is used to measure the throughput,
which is displayed on the UART Console using the settings:

      Baud-rate:  115200
      Data bits:       8
      Stop bits:       1
      Parity:       None
      Flow Control: None

 * The green LED on the Ethernet Jack (PN0) is used as a "blinking LED" to indicate that the application is running.
 * The orange LED (PN1) is used as a "status LED" that turns on when the test is completed successfully.