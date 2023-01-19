## Example Summary

This application demonstrates how to build an HTTP server. The application
provides an example of one possible way to serve static pages from the
Network Services HTTP server.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.


* `EMAC`      Connection to network

## BoosterPacks, Board Resources & Jumper Settings
Please refer to the development board's Hardware User's Guide.

Python 3.X is required for this example. To install Python download the latest
version at https://www.python.org/downloads/release

The Python Requests library is used by the client to send HTTP requests to the
HTTP server. The version used in developing the example was 2.20.1.

The Python urllib3 library is also used. The version used in development was
1.24.1.

The use of these versions of the libraries or higher is recommended. Import
issues may arise otherwise.

## Example Usage

* Example output is generated through use of Display driver APIs. Refer to the
Display driver documentation found in the SimpleLink MCU SDK User's Guide.

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
Homepage"), etc.) to the appropriate COM port.
    * The COM port can be determined via Device Manager in Windows or via
`ls /dev/tty*` in Linux.

* The connection should have the following settings:
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

* In order for the server to have static pages available to
serve, the `make-memzip.py` command must be run on a directory of files
before the application is run. See the [Design Details - Serving Static
Pages](#Serving-Static-Pages) section of this document for instructions.

    * For Code Composer Studio users:

        After importing the example into CCS, right-click the example in the
        `Project Explorer` View and open the project properties. Go to the
        `Build` or `CCS Build` section of properties. Click the `Steps` tab.
        Copy the below into the `Pre-build steps` section and replace the
        location of the Python executable with the path of your Python3
        installation

        ```Python
        # Replace C:\Python3\python.exe below

        C:\Python3\python.exe ${PROJECT_LOC}/make-memzip.py -c ${PROJECT_LOC}/memzip_files.c ${PROJECT_LOC}/targetfs;
        ```

        Build the example. It will execute the pre-build step of converting files
        found in the _targetfs_ directory of your project into an array of data that
        the HTTPServer can serve.

        This is explained more generally in the [Design Details - Serving
        Static Pages](#Serving-Static-Pages) section of this document, so
        that users of different environments can replicate this process.

* Run the example. It begins by starting the network stack. When the stack
receives an IP address from the DHCP server, the IP address is written to the
console. **Note the IP address received.**

* The example uses a Python script along with the Python Requests library to
send HTTP/S requests to the server. The httpSampler script is found in
**&lt;SDK_INSTALL_DIR&gt;/tools/examples/httpSampler.py**

* The example starts up two HTTPServer instances. One is a standard
server for handling HTTP requests. The other is provided with security
certificates and serves HTTPS requests over TLS.

* The example starts up an instance of an HTTPServer that will respond to
requests. Any HTTP client can be used to send requests to the server at the IP
address assigned to the board.

Requests that the server contains a URLHandler for will be
responded to as programmed. Requests without a matching URLHandler will
result in a `Failed: 404 Not Found` (as programmed).

* Run the httpSampler python script

    ```
     python httpSampler.py [-h] [-s] [-u URL] <IP Address>
    ```

### Example:


```python
 python httpSampler.py 192.168.1.100

      200 - {'Content-Length': '42', 'Content-Type': 'text/plain'}
      (raw text)
      Data string successfully posted to server.


      200 - {'Content-Length': '42', 'Content-Type': 'text/plain'}
      (raw text)
      Data string successfully posted to server.


      200 - {'Content-Length': '3', 'Content-Type': 'text/plain'}
      (raw text)
      Tex


      200 - {'Content-Length': '8', 'Content-Type': 'text/plain'}
      (raw text)
      sleeping


      200 - {'Content-Length': '68', 'Content-Type': 'text/plain'}
      (raw text)
      URLHandler #2 : /put This is the body of the resource you requested.


      405 - {'Content-Length': '41', 'Content-Type': 'text/plain'}
      (raw text)
      This method is not supported at this URL.


      Example complete

 python httpSampler.py 192.168.1.100 -s
      Making secure requests
      # The output is the same
```

* You can also run the httpSampler script with the `-u` option to specify a
specific URL to the server. This should be used to view the static pages
originating in the `targetfs` directory.
```
python httpSampler.py 192.168.1.100 -u /index.html
```

## Creating new Certificates

This example ships with one ssl/tls certificate chain created for the example server.

To make your own certificate chain follow the directions below:

* Install OpenSSL from http://www.openssl.org

* Create a directory somewhere on your computer to store your certificates

* Create a root CA certificate:
```
    openssl req -newkey rsa:1024 -sha256 -keyout serverCaKey.pem -out caRequest.pem

    openssl x509 -req -in caRequest.pem -sha256 -signkey serverCaKey.pem -out serverCaCert.pem -days [number of days until certifcate expiration]
```

* Create a server certificate:
```
    openssl req -newkey rsa:1024 -sha256 -keyout serverKey.pem -out serverRequest.pem

    openssl x509 -req -in serverRequest.pem -sha256 -CA serverCaCert.pem -CAkey serverCaKey.pem -CAcreateserial -out serverCert.pem -days [number of days until certifcate expiration]
```

* (If verification of incoming clients is desired) Repeat the steps from the
previous two bullets to create the files for a client certificate chain:
clientCaKey.pem, clientCaCert.pem, clientCert.pem, and clientKey.pem. (ie.
replace `server` with `client` in the commands)

* Decrypt the server key:
```
    openssl rsa -in serverKey.pem -out newServerKey.pem
```

* Edit the httpSrvBasicHooks.c file to match serverCert.pem, serverCaCert.pem, and
newServerKey.pem

* The parameters to the SlNetIf_loadSecObj() and SlNetSock_secAttribSet() calls
inside httpSrvBasic.c will change depending on the names of your created
certificate files. The macros ROOT_CA_CERT_FILE, PRIVATE_KEY_FILE, and
TRUSTED_CERT_FILE will change based on your chosen certificate names.

## Application Design Details

* This application has one primary task:
    * 'serverFxn' creates the HTTP Server and then starts the server by calling
    the HTTPServer_serveSelect() function. This function accepts new connections
    to the server and calls methods to process the incoming requests.

        * If security is enabled, before starting the server, the application loads
        security certificates (example certificates are included with the SDK) and
        calls HTTPServer_enableSecurity() which alerts the server to activate its
        security features. This example server does not verify client certificates,
        though the HTTPServer has the capabilities to do so.

* The example urlhandler provided in `urlsimple.c` is just one example of how a
urlhandler can be designed. From the error code returned in a given
situation, to the status returned to the server after a request that
determines how the server proceeds - these are design decisions influenced by
the HTTP standard.

* `urlmemzip.c` provides an additional example of a urlhandler. This urlhandler
utilizes `memzip.c`, which relies on `make-memzip.py`, to ultimately serve the
files found in `targetfs`.

### Serving Static Pages

The example shown here is _one_ way of serving static web pages from an
HTTPServer application. There are numerous other ways to accomplish this task.

* `make-memzip.py` takes in a directory of files to be served by the
web server. It produces a zip file containing all of the files, still
uncompressed. The zip file is then converted into an array (`memzip_data`)
which can be seen in the output file `memzip_files.c`. All of the data
contained in this zip file, along with bookkeeping information used by
`memzip.c`, is contained in this array.

* `memzip.c` provides the function `memzip_locate` which locates a given
filename inside `memzip_data` and fills a pointer `pData` with the
original file's content. This pointer becomes the body of HTTP responses
crafted by `urlmemzip.c` in `URLMemzip_process`.

* To utilize the functionality provided by `urlmemzip.c`, create a directory
containing web page files and other server resources - similar to the
_targetfs_ directory in this example. The `make-memzip.py` script can then
be run to prepare the file contents to be served by the server.

    * For usage details run
        ```
        python make-memzip.py -h
        ```

    * If using CCS, the provided Pre-build step should be edited with the path
    of a local Python3 installation and the path of the directory containing
    the files to be served.
        ```Python
        <Path to Python executable> ${PROJECT_LOC}/make-memzip.py -c ${PROJECT_LOC}/memzip_files.c <Path to static resources>
        ```

### Kernel Project Configuration

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

## Acknowledgements

The memzip files (memzip.h, memzip.c, and make-memzip.py) were sourced from
https://github.com/micropython/micropython (c3095b37e96aeb69564f53d30a12242ab42bbd02)
and slightly modified by Texas Instruments. Texas Instruments also added the
MIT license to the top of these files in an effort to clarify their licensing.

