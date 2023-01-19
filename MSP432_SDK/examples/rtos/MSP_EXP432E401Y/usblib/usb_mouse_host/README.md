## Example Summary

Sample application to get updates from a mouse HID device.

## Peripherals Exercised

* `Board_LED0`  -    Primary Button Indicator LED
* `Board_LED1`  -    Secondary Button Indicator LED
* `Board_USBHOST`  - Used as HID mouse host*

## Resources & Jumper Settings

Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.

| Development board | Notes |
| --- | --- |
| MSP_EXP432E401Y        | Please ensure that the HID mouse device is connected   |
|        | to your board's USB Host port.   To create a host port use a  |
|        | OTG connector.  Verify that jumpers JP6 and JP7 are jumpered to |
|        | enable host mode 


If an unknown USB device or a power fault on the USB bus has been detected, a
System_printf message is generated.

## Example Usage

Run the example. `Board_LED0` turns ON to indicate TI-RTOS driver
initialization is complete.

To the USB Micro A/B connector port on the board, connect an adapter where one end is micro male type connector and the other end has an USB female receptacle.  Connect a mouse to the adapater.

Once the enumeration occurs, `Board_LED0` and `Board_LED1` show the statuses
of the Primary and Secondary HID mouse buttons respectively. For example, if
the Primary button is pressed `Board_LED0` will turn ON.  Once Primary button is
release, `Board_LED0` will turn OFF.

The HID mouse's movements are tracked as offsets.  The offsets are printed to
the User UART serial window as listed in Device Manager under Ports.

## Application Design Details

This application uses one tasks:
  'mouseHostTask' performs the following actions:
      Waits for a HID mouse device to be connected to the USB host port.

      Gets the status of the device's buttons and updates the LEDs accordingly.

      Prints the mouse's movement offsets via Display_printf.  

For GCC and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
