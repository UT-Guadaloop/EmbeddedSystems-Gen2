# VCU

This project contains all the code necessary to run the Front Hub unit  for Guadaloop Gen 2 pod.

## What the Front Hub Unit should do

- receive and transmit messages to and from VCU. Periodically send and check acknowledgements to and from VCU

- Sample sensor data of following sensors:
  - accelerometer
  - magnetic field sensor
  - temperature sensor
  - reflection sensor

- send sensor data to VCU

## Files

- **init.c:** contains initializations for hub unit and microcontroller. This includes initializations for sensors, GPIO, interrupt, etc. main method is in this file.

- **vcu_communication.c:** contains methods for communnicating with the VCU.

- **accelerometer.c:** contains everything related to sampling accelerometer.

- **magnetic_field.c:** contains everything related to sampling magnetic field sensors.

- **temperature.c:** contains everything related to sampling temperature sensors.