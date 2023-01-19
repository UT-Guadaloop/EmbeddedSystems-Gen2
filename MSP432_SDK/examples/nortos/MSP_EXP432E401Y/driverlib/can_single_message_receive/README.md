# can_singlemessage_recive

 This example shows the basic setup of CAN in order to receive a single message on the CAN bus. This example expects to receive a CAN message with the ID of 0x100 and length of 8. Depending on the value of byte 0, it will turn ON/OFF LED (PN0). It uses the UART to display the number of messages received and the message itself.

 Note: Requires CAN transceiver and CAN sniffer tool, or another MSP432E running the can_singlemessage_transmit example (this example also requires a CAN transceiver).

