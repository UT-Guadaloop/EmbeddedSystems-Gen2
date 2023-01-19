# can_singlemessage_transmit

 This example shows the basic setup of CAN in order to transmit a single messages on the CAN bus. This example sends a CAN message with ID of 0x100 and length of 8 every time the SW1 (PJ0) is pressed. The data also increments when SW1 is pressed. It uses the UART to display the number of messages transmitted and the message itself.
 
 Requires CAN transceiver and CAN sniffer tool, or another MSP432E running the can_singlemessage_receive example (this example also requires a CAN transceiver).

