/*
 * gap_height.c
 *
 * Authors: Dario Jimenez
 *
 * @file contains all methods and code for handling hap height sensors
 *       as well as gap height data sampling task
 */

/** includes **/

/* guadaloop library includes */
#include <guadaloop/lib/sensors/gap_height.h>

/*
 * @brief initialize gap height sensors
 */
static void gapheight_init(void) {
    //TODO
}

/*
 * @brief sample data from gap height sensors
 *
 * @param array of sensors that will be updated
 */
static void gapheight_sample(gapheight_sensor_t **sensors) {
    //TODO
}
/*
 * @brief task that samples gap height data and places it on can bus queue
 */
extern void *gapheight_task(void *arg) {
    //TODO
}
