# VCU

This project contains all the code necessary to run the VCU for Guadaloop Gen 2 pod.

## What the VCU should do

The VCU should successfully perform these tasks:

- receive and transmit messages to and from remote station via the communication unit.

- receive and transmit message from hub units. Periodically send and check acknowledgements to and from hub units.

- validate sensor data received from hub units.

- activate propulsion and braking.

- send sensor data to remote station

## files

- **init.c:** contains initializations for VCU and microcontroller. This includes initializations for sensors, GPIO, interrupt, etc. main method is in this file.

- **groundstation.c:** contains methods for communnicating with ground station.

- **hubunits.c:** contains methods for communicating with hub units

- **podhealth.c:** contains methods for validating sensor data.
