/*
 * Front_Hub_Variables.h
 *
 *  Created on: Feb 13, 2023
 *      Author: Sid Shyamkumar
 */

#ifndef GUADALOOP_LIB_SENSORS_FRONT_HUB_VARIABLES_H_
#define GUADALOOP_LIB_SENSORS_FRONT_HUB_VARIABLES_H_

#include <FreeRTOS.h>
#include <queue.h>
#define QUEUE_LENGTH 0 //Replace with real size

QueueHandle_t xQueue;
UBaseType_t queuelength;
UBaseType_t size;


#endif /* GUADALOOP_LIB_SENSORS_FRONT_HUB_VARIABLES_H_ */
