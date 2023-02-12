/*
 * i2c_read_write.c
 *
 * Authors: Daniela Caballero, Ty Brinker
 *
 * */
#include <stdint.h>
#include "msp432e401y.h"
#include <guadaloop/lib/i2c/i2c_read_write.h>
/*
 *
 * @brief: functions and data structures to help with i2c communication
 *
 */


void I2C_init(I2C_Settings_t *i2cSettings){

    //turn on clock for I2C
    SYSCTL->RCGCI2C |= (1 << (i2cSettings->i2cModule)) ;

    //turn on clock for GPIO RCGCGPIO
    //enable alternative function on ports
    //enable open drain on port
    //select which alt signal is used on each pin (I2C)
    switch(i2cSettings->i2cModule){
    case I2CModule_0 | I2CModule_5: //Port B
        SYSCTL->RCGCGPIO |= 0x02;
        GPIOB->AFSEL = 1;
        GPIOB->ODR = 1;
        switch(i2cSettings->i2cModule){
        case I2CModule_0:
            GPIOB->PCTL |= 0x2200;
            break;
        case I2CModule_5:
            GPIOB->PCTL |= 0x22;
            break;
        }
        break;
    case I2CModule_1:
        SYSCTL->RCGCGPIO |= 0x040; //Port G
        GPIOG->AFSEL = 1;
        GPIOG->ODR = 1;
        GPIOG->PCTL |= 0x22;
        break;
    case I2CModule_2:
        SYSCTL->RCGCGPIO |= 0x400; //Port L
        GPIOL->AFSEL = 1;
        GPIOL->ODR = 1;
        GPIOL->PCTL |= 0x22;
        break;
    case I2CModule_3|I2CModule_4: //Port K
        SYSCTL->RCGCGPIO |= 0x200;
        GPIOK->AFSEL = 1;
        GPIOK->ODR = 1;
       // GPIOK->PCTL = 0x02;
        switch(i2cSettings->i2cModule){
            case I2CModule_3:
                GPIOK->PCTL |= 0x220000;
                break;
            case I2CModule_4:
                GPIOK->PCTL |= 0x22000000;
                break;
        }
        break;
    case I2CModule_6 | I2CModule_7 | I2CModule_8| I2CModule_9: //Port A
        SYSCTL->RCGCGPIO |= 0x001;
        GPIOA->AFSEL = 1;
        GPIOA->ODR = 1;
        GPIOA->PCTL = 0x02;
        switch(i2cSettings->i2cModule){
        case I2CModule_6:
            GPIOA->PCTL |= 0x22000000;
            break;
        case I2CModule_7:
            GPIOA->PCTL |= 0x220000;
            break;
        case I2CModule_8:
            GPIOA->PCTL |= 0x2200;
            break;
        case I2CModule_9:
            GPIOA->PCTL |= 0x22;
            break;
        }
        break;

    }

    //initialize I2C master
    //set clock speed
    i2cSettings->i2cPort->MCR =  0x00000010;
    //i2cSettings->i2cPort->MTPR = i2cSettings->bitRate;
    i2cSettings->i2cPort->MTPR |= 0x09;

}


uint8_t I2C_read_register(Transaction_t *i2cTransaction, I2C_Settings_t *i2cSettings, uint8_t* readBuffer){
    /*
     * To read a register via I2C
     * 1. Write slave addr to I2CMSA
     * 2. Write register addr to I2CMDR
     * 3. Switch to RX mode
     * 4. Read data from I2CMDR
     *
     * */

    //write register address first
    i2cSettings->i2cPort->MSA = i2cTransaction->slaveAddress << 1;
    i2cSettings->i2cPort->MSA &= ~I2C_MSA_SA_S;
    i2cSettings->i2cPort->MDR = i2cTransaction->regAddress;

    i2cSettings->i2cPort->MCS = I2C_MCS_START|I2C_MCS_RUN;
    //i2cSettings->i2cPort->MCS &= 0xEF; //xxx0x111

    while((i2cSettings->i2cPort->MCS) & I2C_MCS_BUSY != 0){}; //busy bit
    while((i2cSettings->i2cPort->MCS & I2C_MCS_ERROR) >> 1 != 0){ //error bit

             if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){

                 i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
                 return 0;

             }

     }


    //switch to RX
    i2cSettings->i2cPort->MSA |= I2C_MSA_SA_S;

   // i2cSettings->i2cPort->MCS &= 0xE7; //xxx01011
    i2cSettings->i2cPort->MCS = I2C_MCS_START|I2C_MCS_RUN;
    uint8_t i;
    for(i=0; i< i2cTransaction->readCount; i++){

        while((i2cSettings->i2cPort->MCS & I2C_MCS_BUSY) !=0 ){};
        while((i2cSettings->i2cPort->MCS & I2C_MCS_ERROR) >> 1 !=0){

        if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){

                    i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
                    return 0;

            }

        }
        readBuffer[i] = i2cSettings->i2cPort->MDR;
        // i2cSettings->i2cPort->MCS &= 0xE9; // xxx01001
        i2cSettings->i2cPort->MCS = I2C_MCS_RUN;
    }

    //i2cSettings->i2cPort->MCS &= 0xE5; //xxx00101
    i2cSettings->i2cPort->MCS = I2C_MCS_STOP;


    return 1;

}

uint8_t I2C_write_register(Transaction_t *i2cTransaction, I2C_Settings_t *i2cSettings, uint8_t *writeBuffer){
    /*
     * To write a register via I2C
     * 1. Write slave addr to I2CMSA
     * 2. Write register addr to I2CMDR
     * 3. Write data to I2CMDR
     *
     * */

     i2cSettings->i2cPort->MSA |= i2cTransaction->slaveAddress << 1;
     i2cSettings->i2cPort->MSA &= ~I2C_MSA_SA_S;
     //write data to I2CMDR
     i2cSettings->i2cPort->MDR = i2cTransaction->regAddress;
    // i2cSettings->i2cPort->MCS |= 0xEF;
     i2cSettings->i2cPort->MCS = I2C_MCS_START|I2C_MCS_RUN;

     while((i2cSettings->i2cPort->MCS) & I2C_MCS_BUSY != 0){}; //busy bit
     while((i2cSettings->i2cPort->MCS & I2C_MCS_ERROR) >> 1 != 0){ //error bit

         if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){

             i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
             return 0;

         }

     }

     uint8_t i;
     for(i=0; i<i2cTransaction->writeCount; i++){
         i2cSettings->i2cPort->MDR = writeBuffer[i];
         while((i2cSettings->i2cPort->MCS) & I2C_MCS_BUSY != 0){};
         if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR)){
             i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
             return 0;
         }
         i2cSettings->i2cPort->MCS = I2C_MCS_RUN;
     }
     i2cSettings->i2cPort->MCS = I2C_MCS_STOP;

     return 1;
}


void init_Transaction(Transaction_t *i2cTransaction){

    i2cTransaction->writeCount = 0;
    i2cTransaction->readCount = 0;

}

void init_Settings(I2C_Settings_t *i2cSettings){

    switch(i2cSettings->i2cModule){

    case I2CModule_0:
           i2cSettings->i2cPort = I2C0;
           break;
    case I2CModule_1:
           i2cSettings->i2cPort = I2C1;
           break;
    case I2CModule_2:
           i2cSettings->i2cPort = I2C2;
           break;
    case I2CModule_3:
           i2cSettings->i2cPort = I2C3;
           break;
    case I2CModule_4:
           i2cSettings->i2cPort = I2C4;
           break;
    case I2CModule_5:
           i2cSettings->i2cPort = I2C5;
           break;
    case I2CModule_6:
           i2cSettings->i2cPort = I2C6;
           break;
    case I2CModule_7:
           i2cSettings->i2cPort = I2C7;
           break;

    }

}



