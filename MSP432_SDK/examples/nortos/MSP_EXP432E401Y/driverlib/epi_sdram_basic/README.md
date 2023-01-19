# MSP432E4 example code for EPI to access SDRAM memory

The EPI is configured to access an SDRAM memory at 60MHz. The example programs
 the GPIOs for EPI and configures the EPI. After the initialization is complete,
 perform basic SDRAM Write and Read in 16-bit, 32-bit and 64-bit access and
 measure the throughput using a SysTick Timer. The resulting throughput is 
 displayed on the Console.