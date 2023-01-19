# Ethernet Boot Application-2.

This example application demonstrates the operation of an application
image using the flash based ethernet boot loader. The application uses
the lwIP TCP/IP stack and puts a callback function on UDP Port 9 to 
receive update from a remote PC for boot request. It then jumps to the
boot loader.

For additional details on lwIP, refer to the lwIP web page at:
http://savannah.nongnu.org/projects/lwip/
