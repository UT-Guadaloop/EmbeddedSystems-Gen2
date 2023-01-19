## Example Summary

This example introduces the MQTT Client Server library API and usage.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` (Refer to Board.h to lookup which LED this maps to) is used for status indication:

<table>
  <tr>
    <th>LED indication</th>
    <th>Interpretation</th>
  </tr>
  <tr>
    <td>Solidly on</td>
    <td>Indicate SimpleLink is properly up - Every Reset / Initialize</td>
  </tr>
  <tr>
    <td>Solidly off</td>
    <td>Device connected and working - Only after connection</td>
  </tr>
</table>

* `CONFIG_GPIO_BUTTON_0` - publishes a msg to broker
* `CONFIG_GPIO_BUTTON_1` - resets board and reconnects

## Example Usage

* Connect an Ethernet cable to the Ethernet port on the device.

* The device must be connected to a network with a DHCP server to run this
example successfully.

* Build the project and flash it by using the Uniflash tool, or equivalently, load the device via a debug session on the IDE of your choice.

* Open a serial port session (e.g. 'HyperTerminal','puTTY', 'Tera Term' etc.) to the appropriate COM port - listed as 'User UART'.
The COM port can be determined via Device Manager in Windows or via `ls /dev/tty*` in Linux.

  The connection should have the following connection settings:

      Baud-rate:    115200
      Data bits:         8
      Stop bits:         1
      Parity:         None
      Flow Control:   None


* Run the example by pressing the reset button or by running debug session through your IDE.
 LED 0 turns ON to indicate the Application initialization is complete

* Once the application has completed its initialization and the network processor is up,
  the application banner would be displayed, showing device details:

        ============================================
           MQTT client server Example Ver: 1.1.1
        ============================================

         MAC address: 04:a3:16:45:89:8e

        ============================================


* At this point the device would attempt to connect to the network and obtain an IP address.
  * Once the connection succeeds, `CONFIG_GPIO_LED_0` would turn on for one second before turning off.

* The application then connects to the remote MQTT broker at 'test.mosquitto.org', and also initializes its own broker instance. You may now interact with it via another MQTT client (e.g. phone or PC app) connected to either the remote broker or the local broker by subscribing/publishing to topics.

* Special handling
    - In case the internal client will disconnect (for any reason) from the remote broker, the MQTT client won't be restarted.
    The user can change that behavior by adding **gResetApplication = true** to *MQTT\_CLIENT\_DISCONNECT\_CB\_EVENT* case in 'Client\_server\_cbs.c' file.

## Application Design Details

* This example provides users the ability to work with both Client and Server (Broker) MQTT entities by combining the two entities into one example. We also allow the user to provide a loop back between internal and external clients (using the enrolled topic)

  - Server/Broker
    - Allows full Broker capabilities (up to 4 clients)
    - Clients can connect/subscribe/publish to the broker at any time

  - Client
    - Allows full mqtt client abilities
    - The Client can connect to the remote broker, subscribe and publish

  - Loopback
    - The internal Client will be connected to the internal Broker, with enrolled topic (subscribed).
    - Any published data on that topic, that arrived to the broker, will be passed to the internal
    client, and will be forwarded to the remote broker.
    - Any topic that the client is subscribed to at the remote broker, that was published, will be sent to the internal broker, and from there to all subscribed clients
    This will allow full forwarding of topics.

The application has two MQTT entities
### MQTT Client Role - Can connect to remote broker

* In order to activate this role, ENABLE\_CLIENT must be defined

####  Remote Broker Configuration
  - Broker parameters can be configured in Mqtt\_ClientCtx parameter which can be found in 'mqtt\_server\_app.c'
  - The broker parameters are:
  - Connection types and security options
    - IPv4 connection
    - IPv6 connection
    - URL connection
    - Secure connection
    - skip domain name verification in secure connection
    - skip certificate catalog verification in secure connection
  - Server Address: URL or IP
    - Port number of MQTT server
    - Method to tcp secured socket
    - Cipher to tcp secured socket
    - Number of files for secure transfer
    - The secure Files

#### Secured socket
  - In order to activate the secured example, SECURE\_CLIENT must be defined. By default the application uses the secured broker at 'test.mosquitto.org'.
    - On platforms such as MSP432e4, where there is no network processor, certificates can be provided in PEM format by adding a secure object via syscfg by doing the following (This is already done for the user in this example):
     1. Open the mqttclient.syscfg file in CCS.
     2. Click on the Network Interfaces module under the IP NETWORK SERVICES (NS) heading.
     3. Check the box next to "Enable Secure Sockets" and add 1 secure object.
     4. In the "Secure Object Variable" field name the variable "externalCAPem". In the "Secure Object Variable Size" field name the variable "externalCAPemLen". These names will correspond with variables we define in the project.
     5. Download the "mosquitto.org.crt" certificate provided in PEM format from the test.mosquitto.org website (the file has extension .crt).
     6. Open this file in a text editor, then copy and paste its contents into the indicated space in the code snippet below. This code snippet is found in `mqttClientServerHooks.c`
        Each line copied from the certificate file should be surrounded with quotes and contain "...\r\n" at the end of each line. The code should follow this format:
```c
        unsigned char externalCAPem[] =
          "-----BEGIN CERTIFICATE-----\r\n"
          "(...certificate line 1...)\r\n"
          "(...certificate line 2...)\r\n"
                      ...
          "-----END CERTIFICATE-----";

        unsigned int externalCAPemLen = sizeof(externalCAPem);
```
    > If you choose to connect to 'test.mosquitto.org', the latter has a link to its certificate authority file in PEM format which can be used to fill `CAPem`.

#### Client Authentication
  - In order to activate the Client authentication by the server, CLNT\_USR\_PWD must be defined  ( ClientUsername and ClientPassword must be defined ).
* Topics Configuration
  - The topics can be set by changing the definitions in 'mqtt\_server\_app.c' file
  - The subscription topics can be set in the **SUBSCRIPTION\_TOPICX** definitions
  - The Client is subscribed to the following default topic
      - **"/Broker/To/simplelink"**
  - The publish topic and data can be set in the **PUBLISH\_TOPICX** and **PUBLISH\_TOPICX\_DATA** definitions
  - The Client publishes the following default topic "/simplelink/ButtonPressEvt0" -
                the topic will be published by pressing `CONFIG_GPIO_BUTTON_0`

### MQTT Server Role - Broker that is ready for external client connection

* In order to activate this role, ENABLE\_SERVER must be defined
* Local Broker Configuration
  - Broker parameters can be configured in Mqtt\_Server parameter which can be found in 'mqtt\_server\_app.c'
  - The broker parameters are:
    - Port number of MQTT server
    - Method to tcp secured socket
    - Cipher to tcp secured socket
    - Number of files for secure transfer
    - The secure Files

#### Secured socket
  - In order to activate the secure local broker, SECURE\_SERVER must be defined. This will allow clients to securely communicate with the broker after authenticating the broker.
  - The certificates used in the example can be found in mqttClientServerHooks.c
    - Clients that want to authenticate the local broker with the example certificate configuration should use the certificate labeled "serverCaCert.pem" 

* Client Authentication
  - In order to activate the Client authentication by the server, SRVR\_USR\_PWD must be defined  ( ClientUsername and ClientPassword must be defined ).

#### Topics Configuration
  - The topics can be set by changing the definitions in 'mqtt\_server\_app.c' file
  - The server can subscribe to a topic (loopback topic). This topic can be set in the **ENROLLED\_TOPIC** definition
  - The Server is subscribed to the following default topic
      - **"/simplelink/To/Broker"**
  - Each client connected to the Broker, can publish on that topic, and the internal client will forward that topic to the remote broker


Both MQTT roles have internal loopback to allow topic forwarding.

## References

[MQTT Org - MQTT Home page](http://mqtt.org/documentation)
[MQTT v3.1.1 specification](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)
[MQTT v3.1 specification](http://www.ibm.com/developerworks/webservices/library/ws-mqtt/index.html)
