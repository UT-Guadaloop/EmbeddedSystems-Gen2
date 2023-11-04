/*
 * AdafruitVL53L0X.c
 *
 *  Created on: Oct 11, 2023
 *
 */

#include <guadaloop/drivers/sensors/distance/AdafruitVL53L0X.h>
#include <stdint.h>
#include <ti/devices/msp432e4/driverlib/driverlib.h>

#define ONE_BYTE 1
#define TWO_BYTES 2


//sensor address
#define VL6180X_ADDRESS 0x29

//sensor registers
#define IDENTIFICATION__MODEL_ID 0x000
#define IDENTIFICATION__MODEL_REV_MAJOR 0x001
#define IDENTIFICATION__MODEL_REV_MINOR 0x002
#define IDENTIFICATION__MODULE_REV_MAJOR 0x003
#define IDENTIFICATION__MODULE_REV_MINOR 0x004
#define IDENTIFICATION__DATE_HI 0x006
#define IDENTIFICATION__DATE_LO 0x007
#define IDENTIFICATION__TIME_MSB 0x008
#define IDENTIFICATION__TIME_LSB 0x009
#define SYSTEM__MODE_GPIO0 0x010
#define SYSTEM__MODE_GPIO1 0x011
#define SYSTEM__HISTORY_CTRL 0x012
#define SYSTEM__INTERRUPT_CONFIG_GPIO 0x014
#define SYSTEM__INTERRUPT_CLEAR 0x015
#define SYSTEM__FRESH_OUT_OF_RESET 0x016
#define SYSTEM__GROUPED_PARAMETER_HOLD 0x017
#define SYSRANGE__START 0x018
#define SYSRANGE__THRESH_HIGH 0x019
#define SYSRANGE__THRESH_LOW 0x01A
#define SYSRANGE__INTERMEASUREMENT_PERIOD 0x01B
#define SYSRANGE__MAX_CONVERGENCE_TIME 0x01C
#define SYSRANGE__CROSSTALK_COMPENSATION_RATE 0x01E
#define SYSRANGE__CROSSTALK_VALID_HEIGHT 0x021
#define SYSRANGE__EARLY_CONVERGENCE_ESTIMATE 0x022
#define SYSRANGE__PART_TO_PART_RANGE_OFFSET 0x024
#define SYSRANGE__RANGE_IGNORE_VALID_HEIGHT 0x025
#define SYSRANGE__RANGE_IGNORE_THRESHOLD 0x026
#define SYSRANGE__MAX_AMBIENT_LEVEL_MULT 0x02C
#define SYSRANGE__RANGE_CHECK_ENABLES 0x02D
#define SYSRANGE__VHV_RECALIBRATE 0x02E
#define SYSRANGE__VHV_REPEAT_RATE 0x031
#define SYSALS__START 0x038
#define SYSALS__THRESH_HIGH 0x03A
#define SYSALS__THRESH_LOW 0x03C
#define SYSALS__INTERMEASUREMENT_PERIOD 0x03E
#define SYSALS__ANALOGUE_GAIN 0x03F
#define SYSALS__INTEGRATION_PERIOD 0x040
#define RESULT__RANGE_STATUS 0x04D
#define RESULT__ALS_STATUS 0x04E
#define RESULT__INTERRUPT_STATUS_GPIO 0x04F
#define RESULT__ALS_VAL 0x050
#define RESULT__HISTORY_BUFFER_x
#define RESULT__RANGE_VAL 0x062
#define RESULT__RANGE_RAW 0x064
#define RESULT__RANGE_RETURN_RATE 0x066
#define RESULT__RANGE_REFERENCE_RATE 0x068
#define RESULT__RANGE_RETURN_SIGNAL_COUNT 0x06C
#define RESULT__RANGE_REFERENCE_SIGNAL_COUNT 0x070
#define RESULT__RANGE_RETURN_AMB_COUNT 0x074
#define RESULT__RANGE_REFERENCE_AMB_COUNT 0x078
#define RESULT__RANGE_RETURN_CONV_TIME 0x07C
#define RESULT__RANGE_REFERENCE_CONV_TIME 0x080
#define READOUT__AVERAGING_SAMPLE_PERIOD 0x10A
#define FIRMWARE__BOOTUP 0x119
#define FIRMWARE__RESULT_SCALER 0x120
#define I2C_SLAVE__DEVICE_ADDRESS 0x212
#define INTERLEAVED_MODE__ENABLE 0x243

static void VL6180x_setRegister(uint16_t registerAddr, uint8_t sendData, uint8_t byteSize);
static uint8_t VL6180x_getRegister(uint16_t registerAddr, uint8_t byteSize);

//TODO

uint8_t VL6180x_init(){
   uint8_t data = VL6180x_getRegister(SYSTEM__FRESH_OUT_OF_RESET, ONE_BYTE);

   if(data != 1){
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
   VL6180x_setRegister(0x0198, 0x01, ONE_BYTE)
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
    //measured in mm
    VL6180x_setRegister(SYSRANGE__START,0x02, ONE_BYTE); //select single shot mode
    while((VL6180x_getRegister(RESULT__INTERRUPT_STATUS_GPIO, ONE_BYTE) & 0x02)>>1 != 1){};
    uint8_t asl_data = VL6180x_getRegister(RESULT__RANGE_VAL, ONE_BYTE); // 1 byte
    VL6180x_setRegister(SYSTEM__INTERRUPT_CLEAR,0x07, ONE_BYTE);
    return asl_data;
}


/*private functions*/

static void VL6180x_setRegister(uint16_t registerAddr, uint16_t sendData, uint8_t byteSize){
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

static uint8_t VL6180x_getRegister(uint16_t registerAddr, uint8_t byteSize){
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
    }else if(byteSize == TWO_BYTES){
        MAP_I2CMasterSlaveAddrSet(I2C1_BASE, VL6180X_ADDRESS, true);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        uint8_t data_LSB = MAP_I2CMasterDataGet(I2C1_BASE);
        MAP_I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        uint8_t data_MSB = MAP_I2CMasterDataGet(I2C1_BASE);
        receiveData = data_MSB << 8 | data_LSB;
    }
    return receiveData;
}

