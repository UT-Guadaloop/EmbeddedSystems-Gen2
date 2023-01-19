C## Example Summary

Sample application to receive characters from a keyboard HID device.

## Peripherals Exercised

* `Board_LED0` -  Caps Lock Indicator LED
* `Board_LED1` -  Scroll Lock Indicator LED
* `Board_USBHOST` -  Used as HID keyboard host*

## Resources & Jumper Settings

Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.

| Development board | Notes |
| --- | --- |
| MSP_EXP432E401Y | Please ensure that the HID keyboard device is connected|
|        | to your board's USB Host port.   To create a host port use a  |
|        | OTG connector.  Verify that jumpers JP6 and JP7 are jumpered to |
|        | enable host mode                                              |


If an unknown USB device or a power fault on the USB bus has been detected, a
System_printf message is generated. Some keyboards feature integrated USB hubs
which are currently not supported by this example.

## Example Usage

Run the example. Board_LED0 turns ON to indicate TI-RTOS driver
initialization is complete.

Once the enumeration occurs on the host, `Board_LED0` and `Board_LED1` show the
Caps Lock and Scroll lock statuses respectively. For example, press Caps Lock on
your keyboard and `Board_LED0` will toggle.  The Host then updates the status
LEDs on the HID keyboard (if any).

Keys pressed on the HID Keyboard are read by the Host and are printed to the user UART
serial window as listed in Device Manager under Ports

## Application Design Details

This application uses one tasks:

  'keyboardHostTask' performs the following actions:
  
      Waits for a HID keyboard device to be connected to the USB host port.

      Gets the status of the keyboard's buttons and updates the LEDs
      accordingly.

      If USEGETCHAR is defined in the example, characters received from the
      keyboard are printed directly to Display_printf.

      Otherwise, keyboard inputs are printed as character arrays delimited by
      the keyboard's Enter key (a <LF> character).

For GNU and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
