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
#include <guadaloop/lib/sensors/Front_Hub_Variables.h>
#include <guadaloop/lib/sensors/queueData.h>

#define NUM_MAG_SENSORS 2

static magneticsensor_t mag_sensors[NUM_MAG_SENSORS]; //Should this changed to an array of pointers?

/*
 * @brief initialize magnetic sensor(s)
 */
static void magnetism_init(void) {
    //TODO
    //Call initializing functions using drivers
}

/*
 * @brief sample data from the magnetic field sensors
 *
 * @param mag_sensors array of mag sensors where data will be updated
 */
static void magnetism_sample(magneticsensor_t **mag_sensors) {
    //TODO
    // Note: what are we doing with this array of mag sensors?
    int i;
    for(i = 0; i<NUM_MAG_SENSORS; i++){
        mag_sensors[i]->field_strength = 0; //Call driver functions
        mag_sensors[i]->location = FRONT_HUB;
    }
}

/*
 * @brief medium priority task that samples data from magnetic
 *        field sensors and puts the data on can bus queue.
 */
extern void *magneticfield_task(void *arg) {
    //TODO
    // How should data be sent from array?
    magnetism_sample(mag_sensors);
    while(1){
        int i;
        for(i = 0; i<NUM_MAG_SENSORS; i++){
            sensor_t magSen;
            magSen.magneticSensor = mag_sensors[i];
            queueData entry = {HALL_EFFECT, magSen};
            xQueue.sendToBack(xQueue, &entry, 0);
        }
    }
}

