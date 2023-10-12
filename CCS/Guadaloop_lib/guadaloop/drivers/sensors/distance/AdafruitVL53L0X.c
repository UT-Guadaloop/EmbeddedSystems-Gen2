/*
 * AdafruitVL53L0X.c
 *
 *  Created on: Oct 11, 2023
 *      Author: Daniela
 */


#include <guadaloop/drivers/sensors/distance/AdafruitVL53L0X.h>
#include <stdint.h>
#include <ti/devices/msp432e4/driverlib/driverlib.h>

#define ONE_BYTE 1
#define TWO_BYTES 2

void VL6180x_setRegister(uint16_t registerAddr, uint8_t sendData, uint8_t byteSize);
uint8_t VL6180x_getRegister(uint16_t registerAddr, uint8_t byteSize);

//TODO

uint8_t VL6180x_init(){
   uint8_t data = VL6180x_getRegister(SYSTEM__FRESH_OUT_OF_RESET, ONE_BYTE);

   if(data!=1){
       //reset failed
       return 0;
   }

   VL6180x_setRegister(0x0207, 0x01, ONE_BYTE);
   VL6180x_setRegister(0x0208, 0x01, ONE_BYTE);
   VL6180x_setRegister(0x0096, 0x00, ONE_BYTE);
   VL6180x_setRegister(0x0097, 0xfd, ONE_BYTE);
   VL6180x_setRegister(0x00e3, 0x01, ONE_BYTE);
   VL6180x_setRegister(0x00e4, 0x03, ONE_BYTE);
   VL6180x_setRegister(0x00e5, 0x02, ONE_BYTE);
   VL6180x_setRegister(0x00e6, 0x01, ONE_BYTE);
   VL6180x_setRegister(0x00e7, 0x03, ONE_BYTE);
   VL6180x_setRegister(0x00f5, 0x02, ONE_BYTE);
   VL6180x_setRegister(0x00d9, 0x05, ONE_BYTE);
   VL6180x_setRegister(0x00db, 0xce, ONE_BYTE);
   VL6180x_setRegister(0x00dc, 0x03, ONE_BYTE);
   VL6180x_setRegister(0x00dd, 0xf8, ONE_BYTE);
   VL6180x_setRegister(0x009f, 0x00, ONE_BYTE);
   VL6180x_setRegister(0x00a3, 0x3c, ONE_BYTE);
   VL6180x_setRegister(0x00b7, 0x00, ONE_BYTE);
   VL6180x_setRegister(0x00bb, 0x3c, ONE_BYTE);
   VL6180x_setRegister(0x00b2, 0x09, ONE_BYTE);
   VL6180x_setRegister(0x00ca, 0x09, ONE_BYTE);
   VL6180x_setRegister(0x0198, 0x01, ONE_BYTE);
   VL6180x_setRegister(0x01b0, 0x17, ONE_BYTE);
   VL6180x_setRegister(0x01ad, 0x00, ONE_BYTE);
   VL6180x_setRegister(0x00ff, 0x05, ONE_BYTE);
   VL6180x_setRegister(0x0100, 0x05, ONE_BYTE);
   VL6180x_setRegister(0x0199, 0x05, ONE_BYTE);
   VL6180x_setRegister(0x01a6, 0x1b, ONE_BYTE);
   VL6180x_setRegister(0x01ac, 0x3e, ONE_BYTE);
   VL6180x_setRegister(0x01a7, 0x1f, ONE_BYTE);
   VL6180x_setRegister(0x0030, 0x00, ONE_BYTE);


   //interrupt settings
   VL6180x_setRegister(SYSTEM__INTERRUPT_CONFIG_GPIO, (4<<3)|3, ONE_BYTE);
   VL6180x_setRegister(0x0011, 0x10, ONE_BYTE);
   VL6180x_setRegister(0x010A, 0x30, ONE_BYTE);
   VL6180x_setRegister(0x003f, 0x46, ONE_BYTE);
   VL6180x_setRegister(0x0031, 0xFF, ONE_BYTE);
   VL6180x_setRegister(0x0041, 0x63, ONE_BYTE);
   VL6180x_setRegister(0x002e, 0x01, ONE_BYTE);

   return 1;

}

uint8_t VL6180x_getDistance(){
    VL6180x_setRegister(SYSRANGE__START,0x02, ONE_BYTE); //select single shot mode
    while((VL6180x_getRegister(RESULT__INTERRUPT_STATUS_GPIO, ONE_BYTE) & 0x02)>>1 != 1){};
    uint8_t asl_data = VL6180x_getRegister(RESULT__RANGE_VAL, ONE_BYTE); // 1 byte
    VL6180x_setRegister(SYSTEM__INTERRUPT_CLEAR,0x07, ONE_BYTE);
    return asl_data;
}



void VL6180x_initCommunication(){
    //initialization for SDL and SCL pins used, can be modified
    uint32_t systemClock;
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
            120000000);

    MAP_GPIOPinConfigure(GPIO_PG0_I2C1SCL);
    MAP_GPIOPinConfigure(GPIO_PG1_I2C1SDA);
    MAP_GPIOPinTypeI2C(GPIO_PORTG_BASE, GPIO_PIN_1);
    MAP_GPIOPinTypeI2CSCL(GPIO_PORTG_BASE, GPIO_PIN_0);
    GPIOG->PUR |= (GPIO_PIN_1 | GPIO_PIN_0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1)))
    {
    }
    MAP_I2CMasterInitExpClk(I2C1_BASE, systemClock, false);
}

/*private functions*/

void VL6180x_setRegister(uint16_t registerAddr, uint16_t sendData, uint8_t byteSize){
    //send 2 byte address
    MAP_I2CMasterSlaveAddrSet(I2C1_BASE, VL6180X_ADDRESS, false);
    MAP_I2CMasterDataPut(I2C1_BASE, registerAddr & 0xFF);
    MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    MAP_I2CMasterDataPut(I2C1_BASE, (registerAddr & 0xFF00) >> 8);
    MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    //send 1 byte of data
    if(byteSize == ONE_BYTE){
        MAP_I2CMasterDataPut(I2C1_BASE, sendData & 0xFF);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    }
    else{
        MAP_I2CMasterDataPut(I2C1_BASE, sendData && 0xFF);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        MAP_I2CMasterDataPut(I2C1_BASE, (sendData && 0xFF00) >> 8);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    }

}

uint8_t VL6180x_getRegister(uint16_t registerAddr, uint8_t byteSize){
    //send 2 byte address
    MAP_I2CMasterSlaveAddrSet(I2C1_BASE, VL6180X_ADDRESS, false);
    MAP_I2CMasterDataPut(I2C1_BASE, registerAddr & 0xFF);
    MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    MAP_I2CMasterDataPut(I2C1_BASE, (registerAddr & 0xFF00) >> 8);
    MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    uint8_t receiveData = 0;
    if(byteSize == ONE_BYTE){
        MAP_I2CMasterSlaveAddrSet(I2C1_BASE, VL6180X_ADDRESS, true);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        receiveData = MAP_I2CMasterDataGet(I2C1_BASE);
    }if else(byteSize == TWO_BYTES){
        MAP_I2CMasterSlaveAddrSet(I2C1_BASE, VL6180X_ADDRESS, true);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        uint8_t data_LSB = MAP_I2CMasterDataGet(I2C1_BASE);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        uint8_t data_MSB = MAP_I2CMasterDataGet(I2C1_BASE);
        receiveData = data_MSB << 8 | data_LSB;
    }
    return receiveData;
}

