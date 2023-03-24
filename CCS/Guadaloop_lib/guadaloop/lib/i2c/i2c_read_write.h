/**
 * i2c_read_write.h
 *
 * Authors: Daniela Caballero
 *
 * @file: functions and data structures to help with i2c communication
 *
 * Visit MSP432E4 SimpleLink Microcontrollers Technical
 * Reference Manual page 1313 for more information on i2c
 *
 * How to use:
 * .. create instance of I2C_Settings_t specifying I2C_Modules_t and I2C_Speed_t types
 *
 * .. call init_Settings function
 *
 * .. create instance of Transaction_t specifying byteCount,
 * .. slaveAddress, and regAddress
 *
 * .. you will change byteCount and regAddress in same instance of Transaction_t
 * .. when you want read/write a different amount of bytes or read/write to a different
 * .. register in the sensor
 *
 * .. call I2C_read_register or I2C_write_register to begin communication
 */

#ifndef I2C_READ_WRITE_H_
#define I2C_READ_WRITE_H_

#include "msp432e401y.h"
#include <guadaloop/lib/i2c/i2c_read_write.h>

/**
 * enumeration of I2C modules on MSP432
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

/**
 * enumeration of I2C clock rate
 */
typedef enum{
    standardMode = 1, //kbps
    fastMode = 4, //kbps
    fastModePlus = 1000, //Mbps
    highSpeedMode = 3300, //Mbps
}I2C_Speed_t;

/**
 * struct of I2C settings options
 * @see I2C_Modules_t
 * @see I2C_Speed_t
 */
typedef struct{
   I2C0_Type *i2cPort;
   I2C_Modules_t i2cModule;
   I2C_Speed_t bitRate;

}I2C_Settings_t;

/**
 * struct of I2C transaction information
 */
typedef struct{
    uint8_t byteCount; /**< how many bytes to read/write from/to sensor */
    uint8_t slaveAddress; /**< sensor address */
    uint8_t regAddress; /**< sensor register address */

}Transaction_t;

/**
 * initializes I2C port
 * @param i2cSettings as I2C_Settings_t type
 * @see I2C_Settings_t
 */
void I2C_init(I2C_Settings_t *i2cSettings);

/**
 * read from sensor register via I2C
 * 1. write slave addr to I2CMSA
 * 2. write register addr to I2CMDR
 * 3. switch to RX mode
 * 4. read data from I2CMDR
 *
 * @param i2cTransaction a pointer to instance of Transaction_t
 * @param i2cSettings a pointer to instance of I2C_Settings_t
 * @param readBuffer a pointer to byte array
 * @see I2C_Settings_t
 * @see Transaction_t
 * @return 1 if success 0 if fail
 */
uint8_t I2C_read_register(Transaction_t *i2cTransaction, I2C_Settings_t *i2cSettings, uint8_t *readBuffer);

/**
 * write sensor register via I2C
 * 1. Write slave addr to I2CMSA
 * 2. Write register addr to I2CMDR
 * 3. Write data to I2CMDR
 *
 * @param i2cTransaction a pointer to instance of Transaction_t
 * @param i2cSettings a pointer to instance of I2C_Settings_t
 * @param writeBuffer a pointer to byte array
 * @see I2C_Settings_t
 * @see Transaction_t
 * @return 1 if success 0 if fail
 */
uint8_t I2C_write_register(Transaction_t *i2cTransaction, I2C_Settings_t *i2cSettings, uint8_t *writeBuffer);

/**
 * initializes Transaction_t instance
 * @param i2cTransaction a pointer to instance of Transaction_t
 * @see Transaction_t
 */
void init_Transaction(Transaction_t *i2cTransaction);

/**
 * initializes I2C_Settings_t instance
 * @param i2cSettings a pointer to instance of I2C_Settings_t
 * @see I2C_Settings_t
 */
void init_Settings(I2C_Settings_t *i2cSettings);

#endif /* GUADALOOP_LIB_I2C_I2C_READ_WRITE_H */
