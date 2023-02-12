/*
 * i2c_read_write.c
 *
 * Authors: Daniela Caballero
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


#define I2C_MCS_STOP   0x00000004
#define I2C_MCS_START  0x00000002
#define I2C_MCS_RUN    0x00000001
#define I2C_MCS_ADRACK 0x00000004
#define I2C_MCS_ACK    0x00000008
#define I2C_MCS_ERROR  0x00000002

 //TODO:
 // Write I2C_read_register and I2C_write_register for all I2C modules
 // Add option for receiving/sending multiple bytes

void I2C_init(I2C_Settings_t *i2cSettings){
    //turn on clock for I2C
    SYSCTL->RCGCI2C |= (1 << (i2cSettings->i2cModule)) ;

    //turn on clock for GPIO RCGCGPIO
    //enable alternative function on ports
    //enable open drain on port
    //select which alt signal is used on each pin (I2C)
    curI2CMod = i2cSettings->i2cModule;
    switch(i2cSettings->i2cModule){
    case I2CModule_0 | I2CModule_5: //Port B
        SYSCTL->RCGCGPIO |= 0x002;
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
        GPIOK->PCTL = 0x02;
        switch(i2cSettings->i2cModule){
        case I2CModule_3:
            GPIOK->PCTL |= 0x220000;
            break;
        }
        case I2CModule_4:
            GPIOK->PCTL |= 0x22000000;
            break;
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
    switch(i2cSettings->i2cModule){
    case I2CModule_0:
        I2C0->MCR = 0x00000010;
        I2C0->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_1:
        I2C1->MCR = 0x00000010;
        I2C1->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_2:
        I2C2->MCR = 0x00000010;
        I2C2->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_3:
        I2C3->MCR = 0x00000010;
        I2C3->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_4:
        I2C4->MCR = 0x00000010;
        I2C4->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_5:
        I2C5->MCR = 0x00000010;
        I2C5->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_6:
        I2C6->MCR = 0x00000010;
        I2C6->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_7:
        I2C7->MCR = 0x00000010;
        I2C7->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_8:
        I2C8->MCR = 0x00000010;
        I2C8->MTPR = i2cSettings->bitRate;
           break;
    case I2CModule_9:
        I2C9->MCR = 0x00000010;
        I2C9->MTPR = i2cSettings->bitRate;
           break;
    }



}

uint8_t I2C_read_register(Transaction_t *i2cTransaction){
    /*
     * To read a register via I2C
     * 1. Write slave addr to I2CMSA
     * 2. Write register addr to I2CMDR
     * 3. Switch to RX mode
     * 4. Read data from I2CMDR
     *
     * */

    I2C0->MSA |= i2cTransaction->slaveAddress << 1;


    return 0;

}


/**
  * @brief: enables all of the transaction's write data to be sent to slave device
  *
  * @param: pointer to transaction data structure
  *
  * @return: returns error bits (0 is returned if no error in transaction)
  *
  */
uint8_t I2C_write_register(Transaction_t *i2cTransaction){

    switch(i2cSettings->i2cModule){
    case I2CModule_0:
        I2C0->MSA |= i2cTransaction->slaveAddress << 1; // MSA[7:1] is slave address
        I2C0->MSA &= ~0x01; // MSA[0] is set to low to write
        //??? why doesent MSA stuff go into MDR
        I2C0->MCS = I2C_MCS_START | I2C_MCS_RUN; //sending 1st byte by generating start bit and MCS Run I imagine says okay go to Master transmit state and continue clock pulsing until stop bit(master says Im done controlling line for now)
        if(I2C0->MCS & (I2C_MCS_DATAACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){
            I2C0->MCS = I2C0_MCS_STOP; //stop transaction
            return (I2C0->MCS & (I2C_MCS_DATAACK | I2C_MCS_ADRACK | I2C_MCS_ERROR)); //return error bits
        }
        for(int i = 0; i< i2cTransaction->writeCount; i++){
            I2C0->MDR =  (i2cTransaction->writeBuffer[i]) & 0xFF; //writing data into shift register
            while(I2C0->MCS & 0x00000001){} //wait until transaction completed
            if(I2C0->MCS & (I2C_MCS_DATAACK | I2C_MCS_ADRACK | I2C_MCS_ERROR )){ //check if slave acknowledged
                I2C0->MCS = I2C0_MCS_STOP;
                return (I2C0->MCS & (I2C_MCS_DATAACK | I2C_MCS_ADRACK | I2C_MCS_ERROR));
            }
            I2C0->MCS = I2C_MCS_RUN; // keep sending data because in master transmit state
        }
        I2C0->MCS = I2C_MCS_STOP; //stop bit generated, then returns SCL to idle state.
        return (I2C0->MCS & (I2C_MCS_DATAACK | I2C_MCS_ADRACK | I2C_MCS_ERROR)); //shld be 0 if no acknowledgement errors

}


void init_Transaction(Transaction_t *i2cTransaction){

    i2cTransaction->writeBuffer = 0;
    i2cTransaction->writeCount = 0;
    i2cTransaction->readBuffer = 0;
    i2cTransaction->readCount = 0;
   // i2cTransaction.status;
   // i2cTransaction.slaveAddress;

}

void init_Settings(I2C_Settings_t *i2cSettings){

}



