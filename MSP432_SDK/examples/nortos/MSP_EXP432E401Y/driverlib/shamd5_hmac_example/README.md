# SHA/MD5 HMAC Demo

This application demonstrates the use of HMAC operation with the available
algorithms of the SHA/MD5 module like MD5, SHA1, SHA224 and SHA256.

This application uses a command-line based interface through a virtual COM
port on UART 0, with the settings 115,200-8-N-1.

Using the command prompt, user can configure the SHA/MD5 module to select
an algorithm to perform HMAC operation.  User can also enter key and data
values during runtime.  Type "help" on the terminal, once the prompt is
displayed, for details of these configuration.

The following web site has been used to validate the HMAC output for these
algorithms: http://www.freeformatter.com/hmac-generator.html

Please note that uDMA is not used in this example.
