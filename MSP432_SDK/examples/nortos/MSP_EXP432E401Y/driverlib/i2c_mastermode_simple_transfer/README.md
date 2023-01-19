# MSP432E4 Example Project for I2C-Master for Simple Write and Read.

This application example configures the I2C module for master mode operation with standard speed.
 The use of the example requires another MSP-EXP432E401Y board to be running the 
 i2c_slavemode_simple_transfer application. The master board sends a data to the slave and the 
 slave inverts the bits. The master board reads the data from the slave and compares the read
 data from the slave with the inverted master data. If there is an error in transmission the
 LED D2 is switched ON.
