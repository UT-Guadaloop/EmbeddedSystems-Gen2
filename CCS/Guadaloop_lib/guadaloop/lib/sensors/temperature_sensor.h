/*
 * temperature_sensor.h
 *
 * Authors: Dario Jimenez
 *
 * @file contains high level data structures to model temperature
 *       sensors
 */

#ifndef GUADALOOP_DRIVERS_TEMPERATURE_SENSOR_H_
#define GUADALOOP_DRIVERS_TEMPERATURE_SENSOR_H_

/** includes **/

/* standard library includes */
#include <stdint.h>

/*
 * temperature typedef
 */
typedef int16_t temp_t;

/*
 * enumaration indicating locations of the temperature sensors
 */
typedef enum temperature_sensor_location {
    //TODO: add locations here
    FRONT_HUB,
    CENTER_HUB,
    REAR_HUB
}tempsensor_loc_t;

/*
 * struct that models temperature sensor
 */
typedef struct temp_sensor {
   temp_t temperature;
   tempsensor_loc_t location;
    //TODO: add/change anything else as needed
}tempsensor_t;

#endif /* GUADALOOP_DRIVERS_TEMPERATURE_SENSOR_H_ */
