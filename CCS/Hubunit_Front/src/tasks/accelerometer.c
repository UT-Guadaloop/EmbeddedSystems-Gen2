/*
 * accelerometer.c
 *
 * Authors: Dario Jimenez
 *
 * @file contains all the necessary methods and code for the handling of
 *       accelerometer in the hub unit as well as the task code for it.
 */

/** INCLUDES **/
#include <stdint.h>

#include <guadaloop/lib/sensors/accelerometer.h>


/*
 * @brief initialize accelerometer
 */
static void acceleratometer_init() {
    //TODO
}

/*
 * @brief sample data from accelerometer and update sensor param
 *
 * @param pointer to accelerometer struct to its value can be updated
 */
static void acceleratometer_sample(accelerometer_t *sensor) {
    //TODO
}

/*
 * @brief high priority task that samples accelerometers
 *
 * @note use above functions in here
 */
extern void *accelerometer_task(void *args) {
    //TODO
}

