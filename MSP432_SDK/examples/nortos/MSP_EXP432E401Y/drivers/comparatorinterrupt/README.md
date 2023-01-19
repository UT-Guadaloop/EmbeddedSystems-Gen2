## Example Summary

This example uses the Comparator driver to detect a voltage threshold and toggle an LED.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Note for this example the positive terminal is used as a reference.
And the input voltage is on the negative terminal. The negative terminal is
determined by the comparator module ID. Additionally, the System Configuration file (\*.syscfg)
present in the project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED0` - Indicates that the driver was initialized within `main()`
* `CONFIG_Comparator0` - Defines the comparator peripheral, `Positive
                         Terminal`, `Negative Terminal`, and `Output Pin`.

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

## Example Usage

* Apply a voltage to the comparator `Negative Terminal`. For quick testing, use
the `GND` pin. Optionally, use a voltage measurement tool on the appropriate
comparator `Output Pin`.

* Run the example

* Change the voltage applied to the comparator `Negative Terminal`.
`CONFIG_GPIO_LED0` will reflect the comparator `Output Pin`.

## Application Design Details

* This example shows how to initialize the Comparator driver, set up
comparator interrupts, and read the current comparator output.

* The comparator generates an interrupt when a rising or falling edge occurs
on the comparator's `Output Pin`.

* The comparator interrupt reads the current output level of the
comparator's `Output Pin` and writes the `CONFIG_GPIO_LED0` to match
that of the comparator output.

TI-RTOS:

* When building in Code Composer Studio, the kernel configuration project will
be imported along with the example. The kernel configuration project is
referenced by the example, so it will be built first. The "release" kernel
configuration is the default project used. It has many debug features disabled.
These feature include assert checking, logging and runtime stack checks. For a
detailed difference between the "release" and "debug" kernel configurations and
how to switch between them, please refer to the SimpleLink MCU SDK User's
Guide. The "release" and "debug" kernel configuration projects can be found
under &lt;SDK_INSTALL_DIR&gt;/kernel/tirtos/builds/&lt;BOARD&gt;/(release|debug)/(ccs|gcc).

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
