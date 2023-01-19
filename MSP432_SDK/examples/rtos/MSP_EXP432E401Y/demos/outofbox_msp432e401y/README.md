---
# outofbox_msp432e401y

---

## Example Summary

The out-of-box demo is a multi-threaded application that records various
board activities by a user, and periodically reports it to an IoT cloud server.
User can also interact with the LaunchPad through a web browser that connects
to the IoT cloud server.

## Peripherals Exercised

* `CONFIG_GPIO_LED0` - Controlled through the IoT cloud portal or command prompt
* `CONFIG_GPIO_LED1` - Used to indicate that data is successfully written to or
read from the IoT Cloud portal
* `CONFIG_GPIO_BUTTON0` - Used to record button presses that is displayed on the IoT
cloud portal
* `CONFIG_GPIO_BUTTON1` - Used to record button presses that is displayed on the IoT
cloud portal
* `CONFIG_UART0` - Used for a simple console
* `EMAC` - Used to connect to the internet
* `Internal Temperature` - Used to read internal temperature that is displayed
on the IoT cloud portal

## Resources & Jumper Settings

> For details about the usage of this demo refer the _SimpleLink Academy_
(http://dev.ti.com/MSP432E4-SimpleLink-Academy) under Labs->LaunchPad Out-of-Box.

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.


## Example Usage

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

* Run the example.

* For details about the usage of this demo refer the _SimpleLink Academy_
(http://dev.ti.com/MSP432E4-SimpleLink-Academy) under LaunchPad Out of Box Experience
