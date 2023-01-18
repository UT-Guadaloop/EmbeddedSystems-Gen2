/*
 * reflection.c
 *
 * Author: Dario Jimenez
 *
 * @file contains all the necessary methods and code for the handling of
 *       reflective sensors in hub init and task for sampling sensor data
 */


/** includes **/

/* guadaloop library includes */
#include <guadaloop/lib/sensors/reflective_sensor.h>


/*
 * @brief initialize reflection sensors
 */
static void reflection_init() {
    //TODO
}

/*
 * @brief sample data from reflective sensors
 *
 * @return number of reflective poles reflective poles reached
 *         since the pod started accelerating
 */
static void reflection_sample(reflectivesensor_t **sensors) {
    //TODO
}

/*
 * @brief calculates the approximate distance the pod has traveled
 *        based on the number of reflective poles passed
 */
static distance_t reflection_distance(reflectivesensor_t *sensors) {
    //TODO
}

/*
 * @brief high priority task that sample reflective sensors periodically.
 */
extern void *reflection_task(void *args) {
    //TODO
}
