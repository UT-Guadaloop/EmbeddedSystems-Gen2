# MSP432E4 example code for EPI to access SDRAM memory using DMA.

The EPI is configured to access an SDRAM memory at 60MHz. The example programs
 the GPIOs for EPI and configures the EPI. After the initialization is complete,
 the EPI is configured to generate a DMA TX request to write an internal buffer
 to the SDRAM memory. On completion the non-blocking read FIFO is configured to
 read the data from the SDRAM to another internal buffer. The resulting data
 check is printed on the console along with the throughput.