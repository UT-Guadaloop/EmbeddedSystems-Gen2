# MSP432E4 Flash Based UART Boot Loader Example.

This example application demonstrates the boot loader implementation over
UART. The execution of the example requires a PC Host running the BSL scripter
and a BSL Rocket connected to the Boot Loader Header on the MSP-EXP432E401Y 
LaunchPad. The BSL scripter downloads the application image over UART with
the MSP432E4 device configured to flash the application image and then jump
 to the application image for execution.
