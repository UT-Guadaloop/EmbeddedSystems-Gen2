## Example Summary

Calculate the CRC of a 32-byte test vector.

### Disclaimer

Texas Instruments does not provide advice on polynomial suitability. The
availability of polynomials in this driver or on a particular device does
not imply fitness for any particular task. If highly reliable error
detection capabilities are required, please consult a domain expert
before choosing a polynomial.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_CRC_0` - For calculating hardware-accelerated checksums
* `CONFIG_UART_0` - Used for example output

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

## Example Usage

* Open a serial session (e.g. `HyperTerminal`,`puTTY`, etc.) to the appropriate
COM port.
    * The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

The connection should have the following settings:
```
    Baud-rate:    115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

* Run the example in debug mode and observe the results.

## Application Design Details

The application calculates the same CRC in two different ways:
* Firstly using the `calculateFull` API, which requires all data to
  be available
* Secondly using the `addData` API, which can run with as few as four bytes at
  a time (for a 4-byte CRC)

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
