/*
 * CAN_message_schema.h
 *
 * Authors: Dario Jimenez
 *
 * @file header file containing methods and data structures for
 *       CAN bus message schema
 */
#include <stdint.h>

#ifndef GUADALOOP_LIB_CAN_MESSAGE_SCHEMA_H_
#define GUADALOOP_LIB_CAN_MESSAGE_SCHEMA_H_

//*Examples**/

//Message from Left LIM temperature sensor number 8 (10001 0010 1000 1000)
//concat(LEFT_LIM, TEMP_SENSE, Sensor8, SensorID)

//Message from Top Right Yoke temperature sensor number 2 (11010 0010 0010 1000)
//concat(TRIGHT_YOKE, TEMP_SENSE, Sensor2, SensorID)

/**Location Macros**/
#define VCU             (1)<<(12)
#define FRONT_HUB       (5)<<(12)
#define CENTER_HUB      (6)<<(12)
#define REAR_HUB        (7)<<(12)
#define BMS             (8)<<(12)
#define COMMS_UNIT      (9)<<(12)
#define ACT_GUI_UNIT    (10)<<(12)
#define LEFT_LIM        (17)<<(12)
#define RIGHT_LIM       (18)<<(12)
#define INVERTER        (19)<<(12)
#define TLEFT_YOKE      (25)<<(12)
#define TRIGHT_YOKE     (26)<<(12)
#define BLEFT_YOKE      (27)<<(12)
#define BRIGHT_YOKE     (28)<<(12)

//**Sensor Type Macros**/
#define IMU             (1)<<(8)
#define TEMP_SENS       (2)<<(8)
#define TOF_SENS        (3)<<(8)
#define ACCELEROMETER   (4)<<(8)
#define PHOTO_SENS      (5)<<(8)
#define HALLEFF_SENS    (6)<<(8)
#define IND_SENS        (9)<<(8)
#define LAS_SENS        (10)<<(8)

//**Sensor Number Macros**/
#define Sensor1         (1)<<(4)
#define Sensor2         (2)<<(4)
#define Sensor3         (3)<<(4)
#define Sensor4         (4)<<(4)
#define Sensor5         (5)<<(4)
#define Sensor6         (6)<<(4)
#define Sensor7         (7)<<(4)
#define Sensor8         (8)<<(4)
#define Sensor9         (9)<<(4)
#define Sensor10        (10)<<(4)
#define Sensor11        (11)<<(4)
#define Sensor12        (12)<<(4)
#define Sensor13        (13)<<(4)
#define Sensor14        (14)<<(4)
#define Sensor15        (15)<<(4)

//Sensor Numbers when message does not involve sensors
#define FHubUnit_SensType           (1)<<(4)
#define CHubUnit_SensType           (2)<<(4)
#define RHubUnit_SensType           (3)<<(4)
#define CommsUnit_SensType          (4)<<(4)
#define VCU_SensType                (8)<<(4)



//*Message ID Macros**/
#define StartID         0
#define StopID          15
#define PropulsionID    2
#define BrakingID       13
#define SensorID        8

#define concat(loc, type, num, id) (loc|type|num|id)


#endif /* GUADALOOP_LIB_CAN_MESSAGE_SCHEMA_H_ */
