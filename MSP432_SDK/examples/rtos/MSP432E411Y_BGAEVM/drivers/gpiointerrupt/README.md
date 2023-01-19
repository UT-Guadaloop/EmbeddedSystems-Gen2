---
# gpiointerrupt

---

## Example Summary

Application that toggles an LED(s) using a GPIO pin interrupt.

## Peripherals Exercised

* `CONFIG_LED_0_GPIO` - Indicates that the board was initialized within
`mainThread()` also toggled by `Board_GPIO_BUTTON0`
* `CONFIG_GPIO_BUTTON_0` - Toggles `CONFIG_LED_0_GPIO`

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.


## Example Usage

* Run the example. `Board_GPIO_LED0` turns ON to indicate driver
initialization is complete.

* `CONFIG_LED_0_GPIO` is toggled by pushing `CONFIG_GPIO_BUTTON_0`.

## Application Design Details

* The `gpioButtonFxn0/1` functions are configured in the *Board.c* file. These
functions are called in the context of the GPIO interrupt.

* There is no button de-bounce logic in the example.

TI-RTOS:

* When building in Code Composer Studio, the configuration project will be
imported along with the example. These projects can be found under
\<SDK_INSTALL_DIR>\/kernel/tirtos/builds/\<BOARD\>/(release|debug)/(ccs|gcc).
The configuration project is referenced by the example, so it
will be built first. The "release" configuration has many debug features
disabled. These features include assert checking, logging and runtime stack
checks. For a detailed difference between the "release" and "debug"
configurations, please refer to the TI-RTOS Kernel User's Guide.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
