/*
 *
 * @file: i2c_read_write.h
 *
 * @brief: functions and data structures to help with i2c communication
 *
 *
 * Written by Daniela Caballero
 *
 * */


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
    void *writeBuffer;
    uint8_t writeCount;
    void *readBuffer;
    uint8_t readCount;
    int status;
    uint8_t slaveAddress;

}Transaction_t;

uint8_t I2C_read_register(Transaction_t i2cTransaction);
uint8_t I2C_write_register(Transaction_t i2cTransaction);
void init_Transaction(Transaction_t i2cTransaction);
void init_Settings(I2C_Settings_t i2cSettings);
