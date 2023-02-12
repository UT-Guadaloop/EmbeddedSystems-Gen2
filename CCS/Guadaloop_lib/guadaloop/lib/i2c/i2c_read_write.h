/*
 * i2c_read_write.h
 *
 * Authors: Daniela Caballero
 *
 * */

#ifndef I2C_READ_WRITE_H_
#define I2C_READ_WRITE_H_
/*
 * @brief: functions and data structures to help with i2c communication
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

typedef struct{
    uint8_t* writeBuffer;
    uint8_t writeCount;
    uint8_t* readBuffer;
    uint8_t readCount;
    int status; // R/W bit
    uint8_t slaveAddress;

}Transaction_t;

/*
 * I2C_init function initializes I2C port
 *
 * create I2C_Settings_t object and set i2cModule
 * and bitRate values then pass as params
 *
 * */
void I2C_init(I2C_Settings_t *i2cSettings);
/*
 * I2C_read_register and I2C_write_register send/receive messages
 * to/from sensor
 *
 * create Transaction_t object and set pointer to buffer of data to be
 * sent/received, how many bytes will be sent/received, and slave address
 * pass as params
 * */
uint8_t I2C_read_register(Transaction_t *i2cTransaction);
uint8_t I2C_write_register(Transaction_t *i2cTransaction);
/*
 *
 * init_Transaction initializes i2cTransaction
 *
 * */
void init_Transaction(Transaction_t *i2cTransaction);
void init_Settings(I2C_Settings_t *i2cSettings);

#endif /* GUADALOOP_LIB_I2C_I2C_READ_WRITE_H */
