/*
 * magnetic_field.h
 *
 * Authors: Dario Jimenez
 *
 * @file contains high level structs and typedefs
 *       to model magnetic field sensors.
 */

#ifndef _MAGNETIC_FIELD_H_
#define MAGNETIC_FIELD_H_

/** includes **/

/* standard library includes */
#include <stdint.h>


/*
 * typedef for magnetic field strength
 */
typedef int tesla_t;

typedef enum magsensor_loc {
    //TODO: add locations here
    SOME_LOC,
}magsensor_loc_t;
/*
 * struct for to model magnetic sensor data
 */
typedef struct magnetic_sensor {
    tesla_t field_strength;
    magsensor_loc_t location;
    //TODO: add required fields
}magneticsensor_t;

#endif /* MAGNETIC_FIELD_H_ */
