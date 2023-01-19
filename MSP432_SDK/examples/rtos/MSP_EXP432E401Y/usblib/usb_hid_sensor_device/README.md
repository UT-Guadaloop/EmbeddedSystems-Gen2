## Example Summary

Sample HID based sensor application that demonstrates sending of device's ADC internal temperature to
host OS.

## Peripherals Exercised

* `Board_LED0` -  Program execution Indicator LED

## Resources & Jumper Settings

Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.

| Development board | Notes |
| --- | --- |
| MSP_EXP432E401Y | Please ensure that the device side of the board is connected|
|        | to your host via a USB cable.     |



## Example Usage

Run the example. Board_LED0 turns ON to indicate TI-RTOS driver
initialization is complete.

USB drivers can be found at the following locations:

    MSP432E USB Drivers:
    Windows USB drivers are located in the products directory:
    <simplelink_msp432e4_sdk\tools\usblib\windows_drivers>


## Application Design Details

This application uses one task:

  'sensorTaskFxn' performs the following actions:
  
      Initializes the Sensor HID device.

      Sends ADC internal temperature data to the USB host via a HID Report.
      

For GNU and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
