# MSP432E4 Example Project for I2C-Master for Burst Write and Read using I2C FIFO and CPU.

This application example configures the I2C module for master mode operation with standard speed.
 The use of the example requires another MSP-EXP432E401Y board to be running 
 i2c_slavemode_masterfifo_transfer application. The master board sends a 32 bytes to the slave
 using the FIFO. The master board addresses the byte offset it wants to read from the slave device
 to which the slave sends the data back to the master. The master uses the FIFO to receive the
 data from the Slave. The master compares the data byte stream read from the slave. If there is
 an error in transmission the LED D2 is switched ON.
