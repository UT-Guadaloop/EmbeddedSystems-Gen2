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
#include <guadaloop/lib/sensors/Front_Hub_Variables.h>
#include <guadaloop/lib/sensors/queueData.h>
#include <guadaloop/drivers/sensors/acceleration/EVALADXL35X.h>
#include <guadaloop/drivers/sensors/acceleration/MPU6050.h>
#include <guadaloop/lib/high_level_I2C.h>

//Create instance of I2C struct and pass to driver functions

I2C_Settings_t accelSet = {I2CModule_1, standardMode};


/*
 * @brief initialize accelerometer
 */
static void acceleratometer_init() {
    //TODO
    // Initialize eval and mpu using accelSet as param
}

/*
 * @brief sample data from accelerometer and update sensor param
 *
 * @param pointer to accelerometer struct to its value can be updated
 */
static void acceleratometer_sample(accelerometer_t *sensor) {
    //TODO
    //Get the value from the driver function
    sensor->location = FRONT_HUB;
    sensor->acceleration = 0;//Replace with acceleration sample from driver
}

/*
 * @brief high priority task that samples accelerometers
 *
 * @note use above functions in here
 */
extern void *accelerometer_task(void *args) {
    //TODO
    //Write to a queue: where the sensor is located, what the data is
    accelerometer_t accel;
    accelerometer_sample(&accel);
    sensor_t accelSensor;
    accelSensor.accelerometer = accel;
    queueData entry = {accelerometer, accelSensor};
    xQueueSendToBack(xQueue, &entry, 0);
}

