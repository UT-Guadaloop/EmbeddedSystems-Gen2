---
# usbDevCdcSerial

---

## Example Summary

This example application turns the evaluation kit into a CDC device
 when connected to the USB host system.  The application
 supports the USB Communication Device Class, Abstract Control Model to
 by interacting with the USB host via a virtual COM port.  For this example,
 the evaluation kit will enumerate as a CDC device with one virtual
 serial port.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
