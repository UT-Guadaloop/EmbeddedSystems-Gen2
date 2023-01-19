## Example Summary

This example introduces the MQTT Client library API and usage.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` - Used for status indication:

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
  <tr>
    <td>Toggling (Solidly on/off)</td>
    <td>Publish a standard message</td>
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
           MQTT client Example Ver: 1.1.1
        ============================================

         MAC address: 71:ff:74:1c:97:88

        ============================================


* At this point the device would attempt to connect to the network and obtain an IP address.
  * Once the connection succeeds, `CONFIG_GPIO_LED_0` would turn on for one second before turning off.

* The application then connects to the MQTT broker at test.mosquitto.org. You may now interact with it via another MQTT client (e.g. phone or PC app) connected to the same broker by subscribing/publishing to topics.

* Special handling
    - In case the client will disconnect (for any reason) from the remote broker, the MQTT will be restarted.
    The user can change that behavior by deleting **gResetApplication = true** from *MQTT\_CLIENT\_DISCONNECT\_CB\_EVENT* case in 'Client\_server\_cbs.c' file.

## Application Design Details

MQTT Client application is used to demonstrate the client side of the MQTT protocol.
It does that by offering a semi-automatic application.

The application starts by connecting to the network to obtain an IP address.

After connection succeeds, the application invokes the 'MqttClient\_start' function which sets all the parameters that are required to create TCP/TLS/SSL connection to the broker, invokes the 'MQTTClient\_connect(gMqttClient)' function which creates the connection to the broker and then
invokes the 'MQTTClient_subscribe(gMqttClient , subscriptionInfo, SUBSCRIPTION\_TOPIC\_COUNT)' function which subscribes to the client topics which are hard coded in 'mqtt\_client\_app.c'

Now the client can receive published messages from the broker.
In this example the topics in the left will trigger the action on the right

              "/simplelink/ToggleLEDCmdL0" <-------------> toggle LED0
              "/Broker/To/simplelink" <-------------> display a message


The user can invoke more commands by pressing the push buttons on the LaunchPad device:

* When push button 0 is pressed, the device will publish the message that includes the topic and data which is hard coded in 'Mqtt\_client\_app.c' by invoking the 'MQTTClient\_publish' function.

* When push button 1 is pressed, the device will unsubscribe to the topics by invoking 'MQTTClient\_unsubscribe' function, disconnect from the broker by invoking the 'MQTTClient\_delete' function and software reset the device.

The application has one MQTT entity
MQTT Client Role - Can connect to remote broker

#### Remote Broker Configuration
  - Broker parameters can be configured in Mqtt\_ClientCtx parameter which can be found in 'mqtt\_client\_app.c'
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
  - In order to activate the secured example, SECURE\_CLIENT must be defined  ( certificates should be programmed ). By default the application uses the secured broker at 'test.mosquitto.org'.
  - On platforms such as MSP432e4, where there is no network processor, certificates can be provided in PEM format by doing the following:
     1. Open mqttclient.syscfg. Navigate to the Network Services module under
     the IP NETWORK SERVICES (NS) heading.
     2. Check the box next to Enable Secure Sockets
     3. Download the "mosquitto.org.crt" certificate provided in PEM format from the test.mosquitto.org website (the file has extension .crt).
     4. Open this file in a text editor, then copy and paste its contents into the indicated space in the code snippet below. This code snippet is found in `mqttClientHooks.c`
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


#### Client Authentication
  - In order to activate the Client authentication by the server, CLNT\_USR\_PWD must be defined  ( ClientUsername and ClientPassword must be defined ).

#### Topics Configuration
  - The topics can be set by changing the definitions in 'mqtt\_client\_app.c' file
  - The subscription topics can be set in the **SUBSCRIPTION\_TOPICX** definitions
  - The Client is subscribed to the following default topics:
      - **"/Broker/To/simplelink"**
      - **"/simplelink/ToggleLEDCmdL0"**
  - The publish topic and data can be set in the **PUBLISH\_TOPICX** and **PUBLISH\_TOPICX\_DATA** definitions
  - The Client publishes the following default topic "/simplelink/ButtonPressEvt0" -
      the topic will be published by pressing `CONFIG_GPIO_BUTTON_0`

## References

[MQTT Org - MQTT Home page](http://mqtt.org/documentation)
[MQTT v3.1.1 specification](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)
[MQTT v3.1 specification](http://www.ibm.com/developerworks/webservices/library/ws-mqtt/index.html)
