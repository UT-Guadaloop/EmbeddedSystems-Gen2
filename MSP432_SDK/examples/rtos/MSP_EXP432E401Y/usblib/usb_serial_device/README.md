## Example Summary

Sample application to transmit and receive data via USB CDC.

Peripherals Exercised
---------------------
* `Board_LED0`      - Transmit indicator LED
* `Board_LED1`      - Receive indicator LED
* `Board_USBDEVICE` - Used for serial communication*

Resources & Jumper Settings
---------------------------
Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.

| Development board | Notes                                                  |
| --- | --- |
| MSP_EXP432E401Y      | Please ensure that the board is connected to your host |
|        | via a USB cable. A VCOM (virtual COM) port driver may  |
|      | need to be installed.                                  |


Example Usage
-------------
First run the example. `Board_LED0` turns ON to indicate TI-RTOS driver
initialization is complete.

When the application is running, open a serial session (e.g. HyperTerminal,
puTTY, etc.) to the appropriate COM port. In Windows, the COM port will be listed
as 'USB Serial Port'.  Note: the COM port can be determined
via Device Manager in Windows or via ls /dev/tty* in Linux.

Set baud rate to 115200.

Once the connection is made, the board transmits the following text every
two seconds:

    "TI-RTOS controls USB.\r\n"

The `Board_LED0` is toggled when a transmission occurs.

`Board_LED1` toggles once character(s)* are received. 

*  The MSP432 collects a single character before returning.

** Note characters typed into the serial session are not echoed back, so you
   will not see them (unless you enable echo on the host).

USB drivers can be found at the following locations:

    MSP432E USB Drivers:
    Windows USB drivers are located in the products directory:
    <simplelink_msp432_sdk\tools\usblib\windows_drivers>


Application Design Details
--------------------------
This application uses two tasks:

  'transmit' performs the following actions:
      Determine if the device is connected to a USB host.

      If connected, periodically sends an array of characters to the USB host.

  'receive' performs the following actions:
      Determine if the device is connected to a USB host.

      If connected, it prints, via Display_printf, any received data and the
      number of bytes. 

For GNU and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
