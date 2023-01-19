# MSP432E4 Flash Based SSI Boot Loader Example.

This example application demonstrates the boot loader implementation over
SSI. The execution of the example requires a PC Host running the BSL scripter
and a BSL Rocket connected to the Boot Loader Header on the MSP-EXP432E401Y 
LaunchPad. The BSL scripter downloads the application image over SSI with
the MSP432E4 device configured as an SSI Slave. It flashes the application 
image and then jumps to the application image for execution.
