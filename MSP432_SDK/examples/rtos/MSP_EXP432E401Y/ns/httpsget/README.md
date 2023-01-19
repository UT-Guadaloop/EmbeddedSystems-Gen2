## Example Summary

This application demonstrates how to perform an HTTP GET request over TLS.

## Peripherals & Pin Assignments

* `EMAC`      Connection to network

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

## Example Usage

* Example output is generated through use of Display driver APIs. Refer to the
Display driver documentation found in the SimpleLink MCU SDK User's Guide.

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
Homepage"), etc.) to the appropriate COM port.
    * The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

* The connection will have the following settings:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

* Connect an Ethernet cable to the Ethernet port on the device.

* The device must be connected to the internet to run this example successfully.

* The device must be connected to a network with a DHCP server to run this
example successfully.

* The example starts the network stack. When the stack
receives an IP address from the DHCP server, the IP address is written to the
console.

* This example then securely connects to the
"https://ev-root.chain-demos.digicert.com/" server whose certificate is
signed by a trusted root authority: "DigiCert High Assurance EV Root CA".

You can find the "DigiCert High Assurance EV Root CA" certificate at the following link:

    https://ev-root.chain-demos.digicert.com/info/index.html

Copy the contents of the certificate between "BEGIN CERTIFICATE" and
"END CERTIFICATE", and add it to the "ti_net_certificate0" array in
"httpsgethooks.c" marked by "USER STEP".

* The example finally makes an HTTP GET call to
"https://ev-root.chain-demos.digicert.com/" and prints the HTTP response
status, the response body and the number of bytes of data received.

## Application Design Details

* This application uses a task for HTTPS communication:

httpsTask  Syncs the system clock with NTP servers, loads the root CA
           certificate, creates a secure connection to the HTTPS server.
           When the connection is established it makes an HTTP GET method call
           using the request URI. The response status code, header fields and
           body from the HTTPS server are processed to get response status
           code and data. The connection is closed and resources are freed
           before the task exits.

* TI-RTOS:

    * When building in Code Composer Studio, the kernel configuration project will
be imported along with the example. The kernel configuration project is
referenced by the example, so it will be built first. The "release" kernel
configuration is the default project used. It has many debug features disabled.
These feature include assert checking, logging and runtime stack checks. For a
detailed difference between the "release" and "debug" kernel configurations and
how to switch between them, please refer to the SimpleLink MCU SDK User's
Guide. The "release" and "debug" kernel configuration projects can be found
under &lt;SDK_INSTALL_DIR&gt;/kernel/tirtos/builds/&lt;BOARD&gt;/(release|debug)/(ccs|gcc).

* FreeRTOS:

    * Please view the `FreeRTOSConfig.h` header file for example configuration
information.
