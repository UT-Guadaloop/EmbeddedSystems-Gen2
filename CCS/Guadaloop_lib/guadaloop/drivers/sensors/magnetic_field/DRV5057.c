/*
 * DRV5057.c
 *
 *  Created on: Dec 31, 2022
 *      Author: dario
 */

#include <guadaloop/drivers/sensors/temperature/DRV5057.h>
#include "/lib/i2c/i2c_read_write.h"
#include "msp432e401y.h"

#include <guadaloop/drivers/sensors/magnetic_field/DRV5057.h>

//TODO
//Set slave address

//Local Queue, first in first out
int[] queue;

//Data is measured
//Select Port B as init for magnetic sensor
void portBInit(void){
    /* INIT PORTS
     * SYSCTL->RCGCPIO
     * GPIOx -> DIR (Output)
     * GPIOx -> DEN (Digital)
     *
     * Since this measures a PWM, set a constant interrupt to measure the high/low voltage of port
     * The measurement given by this sensor is determined by its duty cycle (given by relative on/off time)*/

    //Set transactions and settings
    Transaction_t transaction = {0, 0, 0}; /*{bytecount, slaveAddr, regAddr}*/
    I2C_Settings_t settings = {0, 0, 0}; /*{*i2cPort, i2cModule, bitrate}*/
    init_Transaction(*transaction);
    init_Settings(*settings);
}

//Port X handler

//Get measurement for magnetic sensor
//Method: every time this is called, add a high/low value to a local queue, the amount of high/low values on the stack determines duty cycle
double getMeasurement(void){
    //Set I2C settings

    //Remove oldest value from queue
    I2C_read_register(transaction,tempSettings,tmpBuf);
    //add to queue

    //Calculate duty cycle from local queue

    //Convert and return value
    return 0.0;

}


