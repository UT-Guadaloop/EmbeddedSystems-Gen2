## Example Summary

Sample application to send a string to a USB host as a mouse HID device.

## Peripherals Exercised

* `Board_LED0` -  Indicator LED
* `Board_BUTTON0`  -  Used to simulate the primary button
* `Board_USBDEVICE` - Used as HID Mouse and CDC device*

## Resources & Jumper Settings

> Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.


| Development Board | Notes |
| --- | --- |
| MSP-EXP432e401Y   | Please ensure that the board is connected to your host |
|                   | via a USB cable. A VCOM (virtual COM) port driver may  |
|                   | need to be installed.                                  |


## Example Usage

Run the example. `Board_LED0` turns ON to indicate TI-RTOS driver
initialization is complete.

The example acts as a mouse to your host; it moves the cursor in a figure eight
pattern.  Pressing `Board_BUTTON0` performs a primary click.

If the moving mouse gets annoying, you can suspend the target by hitting Alt-F8
in CCS.

When the application is running, open a serial session (e.g. HyperTerminal,
puTTY, etc.) to the appropriate COM port. Note: the COM port can be determined
via Device Manager in Windows or via ls /dev/tty* in Linux.  The COM port is listed
as 'USB Serial Command Port' in Windows Manager when the device is connected to the
host.

Once the connection is made, the board transmits the following text every
two seconds:

    "TI-RTOS controls USB.\r\n"

The `Board_LED`0 is toggled whenever a transmission occurs.

`Board_LED1` toggles once character(s)* are received. Typed characters are
printed to the SysMin internal buffer. These can be viewed by halting the
target and looking at SysMin in ROV.

*  The MSP432 family devices collect a single character before returning.

** Note characters typed into the serial session are not echoed back, so you
   will not see them (unless you enable echo on the host).

USB drivers can be found at the following locations:

    MSP432 USB Drivers:
    Windows USB drivers are located in the products directory:
    <simplelink_msp432_sdk\tools\usblib\windows_drivers>


## Application Design Details

This application uses three tasks:

  'mouse' performs the following actions:
      Waits for the device to connected to a USB host.

      Once connected it sends predefined mouse offsets (from the
      mouseLookupTable[]) along with primary click status.

  'transmit' performs the following actions:
      Determine if the device is connected to a USB host.

      If connected, periodically sends an array of characters to the USB host.

  'receive' performs the following actions:
      Determine if the device is connected to a USB host.

      If connected, it prints, via System_printf, any received data and the
      number of bytes. SysMin is used for this example, so the output goes into
      its internal buffer.

The tasks are at different priorities to show the functionality. The mouse Task
is the lowest to make sure it does not impact the other tasks.

For GNU and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
