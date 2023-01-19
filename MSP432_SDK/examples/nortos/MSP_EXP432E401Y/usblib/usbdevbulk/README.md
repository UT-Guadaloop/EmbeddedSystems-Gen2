---
# usbDevBulk

---

## Example Summary

This example provides a generic USB device offering simple bulk data 
transfer to and from the host.  The device uses a vendor-specific class ID 
and supports a single bulk IN endpoint and a single bulk OUT endpoint. 
Data received from the host is assumed to be ASCII text and it is 
echoed back with the case of all alphabetic characters swapped.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
