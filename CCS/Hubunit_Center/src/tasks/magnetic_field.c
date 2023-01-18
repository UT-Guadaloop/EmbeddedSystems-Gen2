/*
 * magnetism.c
 *
 * Author: Dario Jimenez
 *
 * @file contains all the necessary methods and code for the handling of
 *       magnetic field sensors in hub init and task for sampling sensor data
 */

/** includes **/
#include <guadaloop/lib/sensors/magnetic_field.h>

#define NUM_MAG_SENSORS 2

static magneticsensor_t mag_sensors[NUM_MAG_SENSORS];

/*
 * @brief initialize magnetic sensor(s)
 */
static void magnetism_init(void) {
    //TODO
}

/*
 * @brief sample data from the magnetic field sensors
 *
 * @param mag_sensors array of mag sensors where data will be updated
 */
static void magnetism_sample(magneticsensor_t **mag_sensors) {
    //TODO
}

/*
 * @brief medium priority task that samples data from magnetic
 *        field sensors and puts the data on can bus queue.
 */
extern void *magneticfield_task(void *arg) {
    //TODO
}

