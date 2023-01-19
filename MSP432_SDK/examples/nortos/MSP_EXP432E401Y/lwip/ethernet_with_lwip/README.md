# Ethernet with lwIP

This example application demonstrates the operation of the MSP432E4
Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain
an Ethernet address.  If DHCP times out without obtaining an address,
AutoIP will be used to obtain a link-local address.  The address that is
selected will be shown on the UART.

UART0, connected to the ICDI virtual COM port and running at 115,200,
8-N-1, is used to display messages from this application. Use the
following command to re-build any file system files that change.

../../../../../tools/examples/makefsfile/makefsfile.exe -i fs -o enet_fsdata.h -r -h -q

For additional details on lwIP, refer to the lwIP web page at:
http://savannah.nongnu.org/projects/lwip/
