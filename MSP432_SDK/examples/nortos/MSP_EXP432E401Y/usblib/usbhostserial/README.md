---
# usbHostSerial

---

## Example Summary

 This example application demonstrates data transfer between a CDC Host and
 a CDC Serial Device using the MSP_EXP432E401Y evaluation kit.  
 To demonstrate the data transfer the CDC device example, "usbDevCdcSerial",
 should be run on 
 another MSP-EXP432E401Y evaluation kit.   The data transferred from the
 device is displayed on the UART terminal. 

 The example automates the transfer of data between host and device.
 Upon initial connection of the CDC device, the UART terminal will display
 a certain number of 'Press Any Key' prompts and when the application
 sends the character 'j' to the device, the device replies back with
 'Received the key press'.

 UART running at 115,200, 8-N-1,
 is used to display messages from this application.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
