# can_multimessage__transmit__receive

This example shows the basic setup of CAN in order to receive and transmit multiple messages on the CAN bus.
This example periodically (10ms) sends CAN messages with ID's of 0x200 and0x201 each message has a length of 2 and contains the status of SW1 and SW2 (pressed or not pressed).

Byte 1 of the message contains the status of SW1, for example:

	0x200   0xB1    0x00  --> SW1 not pressed
	0x200   0xB1    0x01  --> SW1 pressed

 	0x201   0xB2    0x00  --> SW2 not pressed
 	0x201   0xB2    0x01  --> SW2 pressed

Also this example expects to receive a CAN message with the IDs of 0x300 to 0x303 and depending on the value of byte 1, it will turn ON/OFF their corresponding LED, for example:

	0x300   0xD1    0x00  --> LED1 OFF
	0x300   0xD1    0x01  --> LED1 ON

	0x301   0xD2    0x00  --> LED2 OFF
	0x301   0xD2    0x01  --> LED2 ON

	0x302   0xD3    0x00  --> LED3 OFF
	0x302   0xD3    0x01  --> LED3 ON

 At the same time, this example will toggle LED4 every 1 second and it uses the UART to display if there was an error while transmitting or receiving a CAN frame.

Note: Requires CAN transceiver and CAN sniffer tool