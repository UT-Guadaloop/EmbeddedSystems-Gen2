---
# KentecLCD

---

## Example Summary

In this application, a picture is printed on the Kentec LCD display using using grlib and the kentec320x240x16_ssd2119 driver. After 1 second, the screen is cleared and "Hello World" is printed with three color boxes. Using the touch driver and ADC, the screen will detect if it has been pressed. If the user touches one of the three boxes, the font color of "Hello World" will change to the color of the box. If the user presses outside the boxes, the font color will be reset to white.

## Peripherals Exercised

* Kentec 320x240x15 SSD2119 LCD
* ADC0
* grlib

## Resources & Connection Settings

* See the User Guide for instructions on flashing the MSP432E411Y-BGAEVM
* This example requires an external Kentec display device and the KDK350ADPTR-EVM
* In order to connect the LCD to board, the user will need to make sure that,
    1.	The PS1 switch on the KDK350ADPTR-EVM needs to be at up position (shown in blue oval)
    2.	The connector is a bottom contact (shown in blue rectangle)

![](./LCD_connection.jpg)


## Example Usage
* Run the example
* After the picture is displayed, press the colored boxes to change the font color of "Hello World"

