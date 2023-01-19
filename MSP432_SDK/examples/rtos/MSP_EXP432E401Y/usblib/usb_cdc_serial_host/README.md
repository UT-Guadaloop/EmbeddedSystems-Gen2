## Example Summary

Sample CDC Serial HOST application that sends and receives serial data from a CDC device. A UART debug terminal is used to interact with the CDC device. 

## Peripherals Exercised

* `Board_LED0`  -    Primary LED Indicator
* `Board_USBHOST`  - Used as CDC host*

## Resources & Jumper Settings

Please refer to the development board's specific "Settings and Resources"
section in the Getting Started Guide. For convenience, a short summary is also
shown below.

| Development board | Notes |
| --- | --- |
| MSP_EXP432E401Y        | Please ensure that a CDC device is connected   |
|        | to your board's USB Host port.   To create a host port use a  |
|        | OTG connector.  Verify that jumpers JP6 and JP7 are jumpered to |
|        | enable host mode 


## Example Usage

Run the example. `Board_LED0` turns ON to indicate TI-RTOS driver
initialization is complete.

To the USB Micro A/B connector port on the board, connect an adapter where one end is micro male type connector and the other end has an USB female receptacle.  To the adapter connect another MSP432E401Y board downloaded with 'usbdevcdcserial' nortos based example.

Open a serial terminal pointing to the UART COM port as listed in Device Manager.  Upon connection of the Host CDC to a CDC device, the terminal should first list 'CDC Device Connected' and then 'Press Any Key'.  When the user presses any keyboard key, the CDC device should respond back with 'Received the Key Press' indicating that the data was received by the CDC device.  

Serial terminal settings should be Baud Rate = 115200, Data Bits = 8, Stop Bits = 1, Parity = none.


## Application Design Details

This application uses one task:
  'cdcSerialHostFxn' performs the following actions:
  
      Polls for device data and displays the data on the UART terminal

      Sends the user pressed key to the CDC device

      
For GCC and IAR users, please read the following website for details about
semi-hosting:
    http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting
