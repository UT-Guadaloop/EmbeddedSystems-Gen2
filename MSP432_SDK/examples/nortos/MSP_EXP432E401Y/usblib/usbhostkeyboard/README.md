---
# usbHostKeyboard

---

## Example Summary

 This application demonstrates the handling of a USB keyboard attached to
 the evaluation kit.  The application works on keyboards without hubs.
 Attaching the keyboard to the evaluation kit requires an OTG connector.
 Once attached, text typed on the keyboard will appear
 on the UART.  Any keyboard that supports the USB HID BIOS protocol is
 supported.

 UART0, connected to the virtual COM port and running at 115,200,
 8-N-1, is used to display messages from this application.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
