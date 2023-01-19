# MSP432E4 Boot - Application example for Serial boot loaders of UART, SSI, I2C & USB.

This very simple code example shows how an application code can be downloaded using 
the flash boot loader. On being flashed to Sector-1 of the flash the application code
 initalizes the boot interfaces and then toggles the LED on PN0. When the User Switch SW1
 is pressed, it stops the LED blinking and jumps back to the boot loader for a new firmware 
 image.