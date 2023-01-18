/*
 * accelerometer.h
 *
 * Author: Dario Jimenez
 *
 * @file high level data structures that can be used
 *       to model accelerometers.
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

/** INCLUDES **/

/* standard library includes */
#include <stdint.h>

/*
 * typedef to model acceleration
 */
typedef int16_t acceleration_t;

/*
 * enumaration of all accelerometer locations
 */
typedef enum accelerometer_location {
    FRONT_HUB,
    CENTER_HUB,
    REAR_HUB
}accelerometer_loc_t;

/*
 * struct modeling accelerometer
 */
typedef struct accelerometer {
    acceleration_t acceleration;
    accelerometer_loc_t location;
}accelerometer_t;

#endif /* ACCELEROMETER_H_ */
