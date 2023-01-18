/*
 * reflective_sensor.h
 *
 * Authors: Dario Jimenez
 *
 * @file contains high-level data structures that can be
 *       used for reflective sensors.
 */

/** includes **/

/* standard library includes */
#include <stdint.h>


/** Macros and constants **/
#define DISTANCE_PER_POLE 30        /* distance between each reflective pole. TODO: change this to correct distance */

/*
 * typedef a reflective pole along
 */
typedef uint8_t reflectivepole_t;

/*
 * typedef for distance.
 *
 * @note feel free to change to however distance will be represented
 *       and even change the name to 'meter_t' if meters will be the
 *       unit of distance used
 */
typedef uint16_t distance_t;

/*
 * enumaration that indicates the location of the reflective sensor
 */
typedef enum reflectivesensor_location {
    LEFT,
    RIGHT
}reflectivesensor_location_t;

/*
 * struct that models reflective sensor data
 */
typedef struct reflective_sensor {
    reflectivepole_t poles_reached;
    reflectivesensor_location_t location;
}reflectivesensor_t;
