## Example Summary

Sample application that reads a file system from a USB mass storage class device.  It makes use of FatFs, a FAT file system driver.  It provides a UART console for issuing commands to view and navigate the file file system on the MSC device. 

## Peripherals Exercised

* `Board_LED0`  -    Primary LED Indicator
* `Board_USBHOST`  - Used as MSC host*

## Resources & Jumper Settings

Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.

| Development board | Notes |
| --- | --- |
| MSP_EXP432E401Y        | Please ensure that a MSC device is connected   |
|        | to your board's USB Host port.   To create a host port use a  |
|        | OTG connector.  Verify that jumpers JP6 and JP7 are jumpered to |
|        | enable host mode 


## Example Usage

Run the example. `Board_LED0` turns ON to indicate TI-RTOS driver
initialization is complete.

To the USB Micro A/B connector port on the board, connect an adapter where one end is micro male type connector and the other end has an USB female receptacle.  Connect a MSC device to the adapater.

Open a serial terminal pointing to the UART COM port and type in 'help' at the user prompt
for a list of commands that can be executed. 

Serial terminal settings should be Baud Rate = 115200, Data Bits = 8, Stop Bits = 1, Parity = none.


## Application Design Details

This application uses one tasks:
  'mscHostFxn' performs the following actions:
  
      Waits for a MSC device to be connected to the USB host port.

      Waits for commands to be entered by the user and processes the commands

      Prints the results of user requested commands to terminal window

For GCC and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
