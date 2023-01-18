/*
 * temperature.c
 *
 * Authors: Dario Jimenez
 *
 * @file file contains relevant code for hub units temperature sensors
 */

/** includes **/

/* guadaloop library includes */
#include <guadaloop/lib/sensors/accelerometer.h>

/** macros and constants **/
#define NUM_TEMP_SENSORS 2 /* indicates how many temp sensor hub unit has. TODO: change to correct value */


/*
 * @brief initialize all hub unit's temperature sensors.
 *
 * @note e.g. if hub unit has 2 temperature sensors and we decide
 *       to use mcp9808, then we would initialize those two
 *       mcp9808 temp sensors here.
 */
static void temp_init(void) {
    //TODO
}


/*
 * @brief sample data from temperature sensors and update temp sensor
 *        array
 *
 * @param accelerometer pointer where sampled data will be stored
 */
static void temp_sample(accelerometer_t *accelerometer) {
    //TODO
}

/*
 * @brief medium priority tasks that periodically samples temperature
 *        sensors
 *
 * @note use above methods here
 */
extern void *temperature_task(void *args) {
    //TODO
}

