---
# usbHostMsc

---

## Example Summary

 This example application demonstrates reading a file system from a USB mass
 storage class device.  It makes use of FatFs, a FAT file system driver.  It
 provides a simple command console via the UART for issuing commands to view
 and navigate the file system on the mass storage device.

 The first UART, which is connected to the virtual serial port
 on the evaluation board, is configured for 115,200 bits per second, and
 8-N-1 mode.  When the program is started a message will be printed to the
 terminal.  Type ``help'' for command help.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
