/*
 * high_level_I2C.c
 *
 *  Created on: Feb 11, 2023
 *  Author: Sid Shyamkumar
 */

#ifndef HIGH_LEVEL_I2C_H_
#define HIGH_LEVEL_I2C_H_

/*
 *
 * @brief: data structures to help with i2c communication in high level files
 *
 */

typedef enum{
    I2CModule_0 = 0,
    I2CModule_1 = 1,
    I2CModule_2 = 2,
    I2CModule_3 = 3,
    I2CModule_4 = 4,
    I2CModule_5 = 5,
    I2CModule_6 = 6,
    I2CModule_7 = 7,
    I2CModule_8 = 8,
    I2CModule_9 = 9,

}I2C_Modules_t;

typedef enum{
    standardMode,
    fastMode,
    fastModePlus,
    highSpeedMode,
}I2C_Speed_t;

typedef struct{
   I2C_Modules_t i2cModule;
   I2C_Speed_t bitRate;

}I2C_Settings_t;

