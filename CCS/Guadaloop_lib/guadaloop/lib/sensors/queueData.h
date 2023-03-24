/*
 * queueData.h
 *
 *  Created on: Feb 13, 2023
 *      Author: sidha
 */

#ifndef GUADALOOP_LIB_SENSORS_QUEUEDATA_H_
#define GUADALOOP_LIB_SENSORS_QUEUEDATA_H_

#include "accelerometer.h"
#include "gap_height.h"
#include "magnetic_field.h"
#include "reflective_sensor.h"
#include "temperature_sensor.h"

typedef enum{
    ACCELEROMETER = 0,
    HALL_EFFECT = 1,
    REFLECTION = 2,
    TEMP = 3,
    GAP_HEIGHT = 4,
}sensorID_t;

typedef union{
    accelerometer_t accelerometer;
    magneticsensor_t magneticSensor;
    reflectivesensor_t reflectiveSensor;
    tempsensor_t tempSensor;
    gapheight_sensor_t gapHeightSensor;
}sensor_t;

typedef struct{
    sensorID_t ID;
    sensor_t sensor;
}queueData;

#endif /* GUADALOOP_LIB_SENSORS_QUEUEDATA_H_ */
