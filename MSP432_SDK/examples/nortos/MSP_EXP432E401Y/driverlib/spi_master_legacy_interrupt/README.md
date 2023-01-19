# MSP432E4 Example Project for SSI-Master in Legacy Mode.

This application example configures the SSI module for master mode operation in Motorola Frame Format-0.
 The use of the example requires another MSP-EXP432E401Y board to be running the spi_slave_legacy_interrupt
 application. The master board sends a data to the slave and the slave inverts the bits. The master board
 reads the data from the slave and compares the read data from the slave with the inverted master data.