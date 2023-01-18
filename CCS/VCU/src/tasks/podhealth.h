/*
 * podhealth.h
 *
 * Authors: Dario Jimenez
 *
 * @file contains all the validation tasks for the health of the pod
 */

#ifndef TASKS_INC_PODHEALTH_H_
#define TASKS_INC_PODHEALTH_H_

/*
 * @brief task that validates braking data and determines
 *        when to brake based on that data
 */
void braking_task(void *args);

/*
 * @brief task that validates bms data
 */
void bms_validation_task(void *args);

/*
 * @brief task that validates temperature data
 */
void temperature_validation_task(void *args);

/*
 * @brief task that validates gap height data
 */
void gapheight_validation_task(void *args);

/*
 * @brief task that validates current sensor data
 */
void current_validation_task(void *args);

/*
 * @brief task that validates hall effect sensor data
 */
void halleffect_validation_task(void *args);

#endif /* TASKS_INC_PODHEALTH_H_ */
