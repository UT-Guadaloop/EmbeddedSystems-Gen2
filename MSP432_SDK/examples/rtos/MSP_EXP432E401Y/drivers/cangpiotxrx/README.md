### SysConfig Notice

All examples will soon be supported by SysConfig, a tool that will help you 
graphically configure your software components. A preview is available today in 
the examples/syscfg_preview directory. Starting in 3Q 2019, with SDK version 
3.30, only SysConfig-enabled versions of examples will be provided. For more 
information, click [here](http://www.ti.com/sysconfignotice).

---
# cangpiotxrx

---

## Example Summary

This example demonstrates CAN operation. This example is standalone and each 
node on the bus is a capable transmitter and receiver by running it. It also 
demonstrates how to operate in an RTOS context.

Each node will transmit a CAN frame when the GPIO button is pressed. Each node 
will also be listening for a CAN frame that matches its ID filter and mask 
settings.

## Peripherals Exercised

* `CONFIG_GPIO_LED_0` - toggles every other time this CAN node receives a CAN 
frame with an odd number ID. 
* `CONFIG_GPIO_BUTTON_1` - when pressed, prompts this CAN node to send a CAN 
frame, and also increments the CAN frame ID number. 
* `Display_Type_UART` - used to display the CAN frame IDs this CAN node 
transmits and receives.

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

* This table shows how to connect the device to a transceiver, in this case a
TCAN1042DEVM.

  |Device Pins              |Transceiver Pins        |
  |-------------------------|------------------------|
  |`CONFIG_CAN_0` `*RX`     |`TCAN` `RXD`            |
  |`CONFIG_CAN_0` `*TX`     |`TCAN` `TXD`            |
  |`CONFIG_CAN_0` `5V`      |`TCAN` `VCC`            |
  |`CONFIG_CAN_0` `GND`     |`TCAN` `GND`            |

* The transceiver's CANH, CANL, and GND pins can then be connected to any
other CAN-enabled device.

## Example Usage

* At least two devices are needed to run this example, each with its own CAN 
transceiver (usually separate from the LaunchPads). Each node should be able to 
run this same example. Alternatively, you may use one device/transceiver if 
using a CAN analyzer or sniffer tool.
* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
Homepage"), etc.) to the appropriate COM port.
    * The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

The connection should have the following settings
```
    Baud-rate:  115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

* Each CAN node on the bus running this example will toggle its 
`CONFIG_GPIO_LED_0` every other time it receives an odd number CAN frame ID.
* Each CAN node on the bus running this example can transmit a CAN frame by 
pressing `CONFIG_GPIO_BUTTON_1`.
* With each received frame (that passes the CAN filter), the ID will be 
displayed to the serial session via UART. With each `CONFIG_GPIO_BUTTON_1` press, 
the transmitted frame ID will also be displayed to the serial session via UART.

## Application Design Details

* This example opens one instance of CAN at `Baud Rate = 125000`. One message 
object is initialized with parameters `Filter ID: 0x001` and 
`Filter Mask = 0x001`, and defaults to `BLOCKING` mode and `READWRITE` 
direction. The filter parameters ensure this CAN message object only picks up 
on CAN frames with an odd number ID that appear on the bus.

* A GPIO callback function is armed. Upon pressing the button, it will post to 
the task semaphore.

* A new thread (txThread) is created. It continuously loops, but waits on the 
task semaphore before transmitting a CAN frame onto the bus. After 
transmission, also writes some of the frame to UART Display.

* After starting the txThread, mainThread will continue to loop, waiting to 
read a valid CAN frame. Upon receipt, it writes some of the frame to UART 
Display, and toggles its LED.


TI-RTOS:

* When building in Code Composer Studio, the kernel configuration project will
be imported along with the example. The kernel configuration project is
referenced by the example, so it will be built first. The "release" kernel
configuration is the default project used. It has many debug features disabled.
These feature include assert checking, logging and runtime stack checks. For a
detailed difference between the "release" and "debug" kernel configurations and
how to switch between them, please refer to the SimpleLink MCU SDK User's
Guide. The "release" and "debug" kernel configuration projects can be found
under &lt;SDK_INSTALL_DIR&gt;/kernel/tirtos/builds/&lt;BOARD&gt;/(release|debug)
/(ccs|gcc).

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
