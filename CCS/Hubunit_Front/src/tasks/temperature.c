/*
 * temperature.c
 *
 * Authors: Dario Jimenez
 *
 * @file file contains relevant code for hub units temperature sensors
 */

/** includes **/

/* guadaloop library includes */
#include <guadaloop/lib/sensors/temperature_sensor.h>
#include <guadaloop/lib/sensors/Front_Hub_Variables.h>
#include <guadaloop/lib/sensors/queueData.h>

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
    //Call driver functions to init
}


/*
 * @brief sample data from temperature sensors and update temp sensor
 *        array
 *
 * @param accelerometer pointer where sampled data will be stored
 */
static void temp_sample(tempsensor_t *temp_sensor) {
    //TODO
    temp_sensor->temperature = 0; //call driver function
    temp_sensor->location = FRONT_HUB;//Why not working?
}

/*
 * @brief medium priority tasks that periodically samples temperature
 *        sensors
 *
 * @note use above methods here
 */
extern void *temperature_task(void *args) {
    //TODO
    while(1){ //Infinite Loop
        tempsensor_t temp;
        temp_sample(&temp);
        sensor_t tempSensor;
        tempSensor.tempSensor = temp;
        queueData entry = {TEMP, tempSensor};
        xQueueSendToBack(xQueue, &entry, 0);
    }
}

