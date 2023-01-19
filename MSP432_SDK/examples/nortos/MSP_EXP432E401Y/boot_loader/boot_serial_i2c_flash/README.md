# MSP432E4 Flash Based I2C Boot Loader Example.

This example application demonstrates the boot loader implementation over
I2C. The execution of the example requires a PC Host running the BSL scripter
and a BSL Rocket connected to the Boot Loader Header on the MSP-EXP432E401Y 
LaunchPad. The BSL scripter downloads the application image over I2C with
the MSP432E4 device configured as an I2C Slave with the Slave Address of 0x42.
It flashes the application image and then jumps to the application image for execution.
