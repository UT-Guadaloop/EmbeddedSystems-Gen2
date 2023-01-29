/*
 *
 * i2c_read_write.c
 * Written by Daniela Caballero
 *
 * */
#include <stdint.h>
#include <i2c_read_write_register.h>


void I2C_init(I2C_Settings_t){



}

uint8_t I2C_read_register(Transaction_t i2cTransaction){




}

uint8_t I2C_write_register(Transaction_t i2cTransaction){


}


void init_Transaction(Transaction_t i2cTransaction){

    i2cTransaction.writeBuffer = NULL;
    i2cTransaction.writeCount = 0;
    i2cTransaction.readBuffer = NULL;
    i2cTransaction.readCount = 0;
    i2cTransaction.status;
    i2cTransaction.slaveAddress;

}

void init_Settings(I2C_Settings_t i2cSettings){

    i2cSettings.GPIOport = 0;
    i2cSetting.bitRate = 0;

}


