/*
 * i2c_read_write.c
 *
 * Authors: Daniela Caballero, Ty Brinker
 *
 * @file: functions and data structures to help with i2c communication
 *
 * */
#include <stdint.h>
#include "msp432e401y.h"
#include <guadaloop/lib/i2c/i2c_read_write.h>

/** MACROS **/
#define I2C_INIT_MASTER 0x00000010
#define PORTB_RCGCGPIO 0x02
#define PORTG_RCGCGPIO 0x040
#define PORTL_RCGCGPIO 0x400
#define PORTK_RCGCGPIO 0x200
#define PORTA_RCGCGPIO 0x001
#define I2C_AFSEL_PCTL 0x22

void I2C_init(I2C_Settings_t *i2cSettings){

    SYSCTL->RCGCI2C |= (1 << (i2cSettings->i2cModule));

    switch(i2cSettings->i2cModule){
        case I2CModule_0 | I2CModule_5: //Port B
            SYSCTL->RCGCGPIO |= PORTB_RCGCGPIO;
            GPIOB->AFSEL = 1;
            GPIOB->ODR = 1;
            switch(i2cSettings->i2cModule){
            case I2CModule_0:
                GPIOB->PCTL |= I2C_AFSEL_PCTL << 2;
                break;
            case I2CModule_5:
                GPIOB->PCTL |= I2C_AFSEL_PCTL;
                break;
            }
            break;
        case I2CModule_1:
            SYSCTL->RCGCGPIO |= PORTG_RCGCGPIO; //Port G
            GPIOG->AFSEL = 1;
            GPIOG->ODR = 1;
            GPIOG->PCTL |= I2C_AFSEL_PCTL;
            break;
        case I2CModule_2:
            SYSCTL->RCGCGPIO |= PORTL_RCGCGPIO; //Port L
            GPIOL->AFSEL = 1;
            GPIOL->ODR = 1;
            GPIOL->PCTL |= I2C_AFSEL_PCTL;
            break;
        case I2CModule_3|I2CModule_4: //Port K
            SYSCTL->RCGCGPIO |= PORTK_RCGCGPIO;
            GPIOK->AFSEL = 1;
            GPIOK->ODR = 1;
            switch(i2cSettings->i2cModule){
                case I2CModule_3:
                    GPIOK->PCTL |= I2C_AFSEL_PCTL << 4;
                    break;
                case I2CModule_4:
                    GPIOK->PCTL |= I2C_AFSEL_PCTL << 6;
                    break;
            }
            break;
        case I2CModule_6 | I2CModule_7 | I2CModule_8| I2CModule_9: //Port A
            SYSCTL->RCGCGPIO |= PORTA_RCGCGPIO;
            GPIOA->AFSEL = 1;
            GPIOA->ODR = 1;
            switch(i2cSettings->i2cModule){
            case I2CModule_6:
                GPIOA->PCTL |= I2C_AFSEL_PCTL << 6;
                break;
            case I2CModule_7:
                GPIOA->PCTL |= I2C_AFSEL_PCTL << 4;
                break;
            case I2CModule_8:
                GPIOA->PCTL |= I2C_AFSEL_PCTL << 2;
                break;
            case I2CModule_9:
                GPIOA->PCTL |= I2C_AFSEL_PCTL;
                break;
            }
            break;
    }

    i2cSettings->i2cPort->MCR =  I2C_INIT_MASTER;
    i2cSettings->i2cPort->MTPR = i2cSettings->bitRate;
}


uint8_t I2C_read_register(Transaction_t *i2cTransaction, I2C_Settings_t *i2cSettings, uint8_t* readBuffer){

    i2cSettings->i2cPort->MSA = i2cTransaction->slaveAddress << 1;
    i2cSettings->i2cPort->MSA &= ~I2C_MSA_SA_S;
    i2cSettings->i2cPort->MDR = i2cTransaction->regAddress;

    i2cSettings->i2cPort->MCS = I2C_MCS_START|I2C_MCS_RUN;

    while((i2cSettings->i2cPort->MCS) & I2C_MCS_BUSY != 0){}; //busy bit
    while((i2cSettings->i2cPort->MCS & I2C_MCS_ERROR) >> 1 != 0){ //error bit
             if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){
                 i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
                 return 0;
             }
     }

    i2cSettings->i2cPort->MSA |= I2C_MSA_SA_S;
    i2cSettings->i2cPort->MCS = I2C_MCS_START|I2C_MCS_RUN;
    uint8_t i;
    for(i=0; i< i2cTransaction->byteCount; i++){
        while((i2cSettings->i2cPort->MCS & I2C_MCS_BUSY) !=0 ){};
        while((i2cSettings->i2cPort->MCS & I2C_MCS_ERROR) >> 1 !=0){
            if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){
                    i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
                    return 0;
            }
        }
        readBuffer[i] = i2cSettings->i2cPort->MDR;
        i2cSettings->i2cPort->MCS = I2C_MCS_RUN;
    }

    i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
    return 1;
}

uint8_t I2C_write_register(Transaction_t *i2cTransaction, I2C_Settings_t *i2cSettings, uint8_t *writeBuffer){
     i2cSettings->i2cPort->MSA |= i2cTransaction->slaveAddress << 1;
     i2cSettings->i2cPort->MSA &= ~I2C_MSA_SA_S;
     i2cSettings->i2cPort->MDR = i2cTransaction->regAddress;
     i2cSettings->i2cPort->MCS = I2C_MCS_START|I2C_MCS_RUN;

     while((i2cSettings->i2cPort->MCS) & I2C_MCS_BUSY != 0){}; //busy bit
     while((i2cSettings->i2cPort->MCS & I2C_MCS_ERROR) >> 1 != 0){ //error bit
         if(i2cSettings->i2cPort->MCS & (I2C_MCS_DATACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){
             i2cSettings->i2cPort->MCS = I2C_MCS_STOP;
             return 0;
         }
     }

     uint8_t i;
     for(i=0; i<i2cTransaction->byteCount; i++){
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
    i2cTransaction->byteCount = 0;
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
