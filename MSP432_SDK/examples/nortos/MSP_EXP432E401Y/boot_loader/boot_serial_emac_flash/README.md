# MSP432E4 Flash Based Ethernet Boot Loader Example.

This example application demonstrates the boot loader implementation over
Ethernet. The execution of the example requires a ethernet 10/100 Mbps switch
to which the Target LaunchPad and the PC are connected. The Boot loader 
sends out the BOOTP request. When the BOOTP request is acknowledged by a BOOTP
server running on the PC Host via an application like the BSL Scripter, it 
sends a TFTP request to get the application image from the PC host. It flashes
the application image and then jumps to the application image for execution.
