---
# usbDevCSerial

---

## Example Summary

This example application turns the evaluation kit into a multiple virtual
 serial ports when connected to the USB host system.  The application
 supports the USB Communication Device Class, Abstract Control Model to
 redirect UART0 traffic to and from the USB host system.  For this example,
 the evaluation kit will enumerate as a composite device with two virtual
 serial ports. Including the physical UART0 connection with the ICDI, this
 means that three independent virtual serial ports will be visible to the
 USB host.  The three virtual COM ports will require three serial terminals
 to be opened at baudrate of 115200, data bits: 8, stop bits: 1 parity: none

 The first virtual serial port will echo data to the physical UART0 port on
 the device which is connected to the virtual serial port on the ICDI device
 on this board. The physical UART0 will also echo onto the first virtual
 serial device provided by the controller.

 The second virtual serial port will provide a console that can
 echo data to both the ICDI virtual serial port and the first
 virtual serial port.  It will also allow turning on, off or toggling the
 boards led status.  Typing a "?" and pressing return should echo a list of
 commands to the terminal, since this board can show up as possibly three
 individual virtual serial devices.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
