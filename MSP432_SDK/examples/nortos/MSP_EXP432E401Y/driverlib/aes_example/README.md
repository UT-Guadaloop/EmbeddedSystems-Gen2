# AES Encryption and Decryption Demo

This application demonstrates encryption and decryption for the available
modes of the AES module.

This application uses a command-line based interface through a virtual COM
port on UART 0, with the settings 115,200-8-N-1.

Using the command prompt, user can configure the AES module to select the
mode, key-size, and direction (encryption/decryption) during runtime.  User
can also enter key, data and IV values during runtime.  Type "help" on the
terminal, once the prompt is displayed, for details of these configuration.

The examples from NIST specification at the following link have been used
to validate the AES outptut.
http://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38a.pdf

Please note that uDMA is not used in this example.
