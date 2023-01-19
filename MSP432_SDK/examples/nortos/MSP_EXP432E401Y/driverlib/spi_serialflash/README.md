# spi_serialflash

This Example demonstrate how to configure the QSSI module for accessing a Serial Flash in Adv-Bi-Quad Mode. It uses the UART to display status messages.

This example will send out 256 bytes in Advanced, Bi and Quad Mode and then read the data in Advanced, Bi and Quad Mode. Once the data check is completed it shall Erase the Serial Flash to return the device to it's original state

Note: Requires external Quad Serial Flash (MX66L51235F - 512M-BIT). Similar to the SDRAM-NVM Daughtercard (http://www.ti.com/tool/TIDM-TM4C129SDRAMNVM)
