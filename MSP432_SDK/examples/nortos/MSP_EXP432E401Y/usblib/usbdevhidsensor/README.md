---
# usbDevHidSensor

---

## Example Summary

 This example application turns the evaluation board into a USB Temperature
 Sensor/HID device supporting the Human Interface Device class.  When the
 program is run, the sensor/HID device will send its internal temperature in
 degrees C along with the device's state and event data.  The generic HID
 application tool 'HidDemo_Tool' can be used to display the data.  The tool
 can be found in the following location in the SDK package:
 The device's temperature, state and event is based on the sensor values
 specified in the HID Report descriptor.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
