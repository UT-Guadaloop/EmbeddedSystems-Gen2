# MSP432E4 example code for UART data transfer with DMA.

The UART0 is configured to generate a DMA request when data is received. The DMA controller
 then reads the DATA from the UART controller and sends it back to the Console.