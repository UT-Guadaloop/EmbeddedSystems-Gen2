## Example Summary

This application demonstrates how to use TCP and TLS. The device acts
as a server that accepts connection from a client.

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

Python 3.X is required for this example. To install python download the latest
version at https://www.python.org/downloads/release

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

* The device must be connected to a network with a DHCP server to run this
example successfully.

* The example starts the network and TLS stack. When the stack
receives an IP address from the DHCP server, the IP address is written to the
console.

* Run the tcpSendReceiveTLS python script that is shipped with your SDK.
    * The script is found in:

**&lt;SDK_INSTALL_DIR&gt;/tools/examples/tcpSendReceiveTLS.py**

Usage:

```
python tcpSendReceiveTLS.py <IP-addr> <port> <id> <server CA cert> <client cert> <client key> -l[length] -s[sleep in mS] -n[number of transmits per report]

  <IP-addr>        is the IP address of the device
  <port>           is the TCP port being listened to (1000)
  <id>             is a unique id for the executable. Printed out with a packet transmission report.
                   It allows the user to run multiple instances of tcpSendReceiveTLS.
  <server CA cert> is the server's root certificate file to point to. The default is in
                   SDK_INSTALL_DIR/examples/rtos/MSP_EXP432E401Y/ns/tcpechotls/certs/serverCaCert.pem
  <client cert>    is the client's certificate file to point to, used for authenticating the client. The default is in
                   SDK_INSTALL_DIR/examples/rtos/MSP_EXP432E401Y/ns/tcpechotls/certs/clientCert.pem
  <client key>     is the client's private key file to point to, used for authenticating the client. The default is in
                   SDK_INSTALL_DIR/examples/rtos/MSP_EXP432E401Y/ns/tcpechotls/certs/clientKey.pem

  Optional:
    -l[length]      size of the packet in bytes. Default is 1024 bytes.
    -s[sleep in mS] usleep time to between sends. Default is 0 mSecs.
    -n[number of transmits per report] the number of transmits to occur before being reported onto the console. Default is 100 transmits.
```

  Example:
        **python tcpSendReceiveTLS.py 192.168.1.100 1000 1 certs/serverCaCert.pem certs/clientCert.pem certs/clientKey.pem -s100**

* If prompted for a pass phrase, enter `dummy_client`
* Messages such as the following will begin to appear on the terminal window when a TCP packet has been echoed back:
```
        Starting test with a 100 mSec delay between transmits and reporting every 100 transmit(s)
        [id 1] count = 100, time = 10
        [id 1] count = 200, time = 20
        [id 1] count = 300, time = 30
```

## Creating new Certificates

This example ships with two ssl/tls certificate chains created for the example, one for the server, another for the client.
To make your own set of certificates follow the directions below:

* Install OpenSSL from http://www.openssl.org

* Create a directory somewhere on your computer to store your certificates

* Create root CA certificate:
```
    openssl req -newkey rsa:1024 -sha256 -keyout serverCaKey.pem -out caRequest.pem

    openssl x509 -req -in caRequest.pem -sha256 -signkey serverCaKey.pem -out serverCaCert.pem -days [number of days until certifcate expiration]
```

* Create server certificate:
```
    openssl req -newkey rsa:1024 -sha256 -keyout serverKey.pem -out serverRequest.pem

    openssl x509 -req -in serverRequest.pem -sha256 -CA serverCaCert.pem -CAkey serverCaKey.pem -CAcreateserial -out serverCert.pem -days [number of days until certifcate expiration]
```

* Repeat the steps from the previous two bullets to create the files for the client certificate chain: clientCaKey.pem, clientCaCert.pem, clientCert.pem, and clientKey.pem. (ie. replace `server` with `client` in the commands)

* Decrypt server key:
```
    openssl rsa -in serverKey.pem -out newServerKey.pem
```

* Edit the certificate declarations in the tcpEchoHooks.c file to match clientCaCert.pem, serverCert.pem, serverCaCert.pem, and newServerKey.pem

* serverCaCert.pem, clientCert.pem, and clientKey.pem are what you would pass to the script `tcpSendReceiveTLS.py`

## Application Design Details

This application uses two types of tasks:

1. **tcpHandler** - Loads certificates, creates a socket and accepts
                  incoming connections.  When a connection is established,
                  a TLS session is started and a **tcpWorker** task is
                  dynamically created to send or receive data securely.
2. **tcpWorker**  - Echoes TCP packets back to the client securely.

**tcpHandler** performs the following actions:
   * Load the client's certificate authority and server
      certificates, and server key to the context buffers.
   * Create a socket and bind it to a port (1000 for this example).
   * Wait for incoming requests.
   * Once a request is received, a TLS session is started,
     client authentication is performed,
     and a new tcpWorker task is dynamically created to manage the
     the communication (echo encrypted TCP packets).
   * Waiting for new requests.

**tcpWorker** performs the following actions:
   * Allocate memory to serve as a TCP packet buffer.
   * Receive data from socket client (Data decrypted by TLS layer before it
     is accessible by the user application).
   * Echo back TCP packet to the client (Data encrypted by TLS layer before
     sending to the client).
   * When client closes the socket, close server socket,
     free TCP buffer memory and exit the task.

* This example can be compared to the tcpEcho example to see the TLS layer that
is added to make the TCP communication secure.

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
