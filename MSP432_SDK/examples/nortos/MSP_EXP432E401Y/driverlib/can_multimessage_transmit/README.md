# can_multimessage_transmit

This example shows the basic setup of CAN in order to transmit multiple messages on the CAN bus at different rate. This example periodically sends CAN messages with ID's of 0x100(10ms), 0x101(20ms), 0x102(100ms) and 0x103(1 second) and each message has a length of 8. It uses the UART to display if there was an error while transmitting the CAN frame.

Note: Requires CAN transceiver and CAN sniffer tool