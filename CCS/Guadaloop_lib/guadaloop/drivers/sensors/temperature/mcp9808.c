/*
 * mcp9808.c
 *
 *@brief Contains function to read temperature data and enables temperature monitoring.
 *
 *
 *Note: Must call I2C_Init() before calling MCP9808_Init function
 *
 * Created on: Dec 31, 2022
 * Author: TY Brinker
 */

#include <guadaloop/drivers/sensors/temperature/mcp9808.h>
#include "/lib/i2c/i2c_read_write.h"
#include "msp432e401y.h"

#define MSP9898_SlaveAddress 00110000 //can be used as slave addy to r/w

//MSP9808 register offsets
#define CONFIG_REG 00000001
#define TMP_UPPERLIM_REG 00000010
#define TMP_LOWERLIM_REG 00000011
#define TMP_CRITICALBOUNDARY_REG 00000100
#define RESOLUTION_REG 00001000
#define CURRENT_TEMP_REG 00000101

///MSP9808 register bit settings
#define CONFIG_MSB 00000110 //hysteresis set to 6 °C
#define CONFIG_LSB 01011110 //comparator's output(Tcur > Tcrit) enabled as alarm output, Tupper & Tlower regs cant be written to
#define TMP_CRITBOUNDARY_MSB 00000111  //setting critical bound tmp to 121 °C
#define TMP_CRITBOUNDARY_LSB 10010000
#define RESOLUTION_BITS 00000001 //set .25 °C resolution. idk if this is gonna give error bc can only write to last two bits here(See data sheet), but need this in 8 bits bc thats how data sent in i2c

/*
 * @brief: Configures pin 4 on Port C to support falling and rising edge detection
 *
 */
void PORTC_Init(void){
 SYSCTL->RCGCGPIO |= 0x04; //enabling port C clock bc writing to registers on this port below
 //initialize semaphore? idk if needed yet(look back at ur lab3)
 GPIOC->DIR &= ~0x10; //PC4 set to input
 GPIOC->DEN = 0x10; //digital input on PE4
 GPIOC->IS &= ~0x10; //clearing bit so pin detects edges
 //GPIOC->IEV //-> u might need to set or clear... but i think if u dont set anything, but set IBE then it will detect both edges
 GPIOC->IM &= ~0x10; //disable PE4 edge triggered interrupts... clear the correpsonding pin's bit in this reg
 //configure pin to support interrupts on both rising and falling edge
 GPIOC->ICR = 0x10; //RIS bit cleared in GPIORIS register by writing 1 to correpsonding bit in GPIO Interrupt Clear Register(GPIOICR)--> potentially rising edge occurs during system power up
 GPIOC->IBE |= 0x10; //set PE4 in this register to detect interrupts on both falling and rising edge
 NVIC_PRI0_R = (NVIC_PRI0_R&0xFF00FFFF)|0x00200000;  //set priority of interrupt on bits 23-21 bc setting priority of PortC Handler(8 bit field ur setting to priority 1 )
 GPIOC->IM |= 0x10;  //enable edge triggered interrupts on PE4
 }





/*
 * @brief: Initializes MCP9808 temperature sensor to raise alert signal when Tcur > Tcrit
 *         have an Hysteresis set to 6 °C, and to measure temperatures with 0.25 °C resolution
 *
 */
void MCP9808_Init(void){
    Disable_Interrupts(); //might have to write this?
    PortC_Init(); //enabling PC4 to receive alarm output signal before initializing MCP9808
    //call I2C Init func? feel like this is initalized before this tho
    uint8_t initData[20] = {CONFIG_MSB, CONFIG_LSB, TMP_CRITBOUNDARY_MSB,TMP_CRITBOUNDARY_LSB,RESOLUTION_BITS}; //data for all register initialization
    //I2C_Setting_t tmpSettings = {};
    Transaction_t curTransaction = {2,0,MSP9898_SlaveAddress,CONFIG_REG};
    //I2C_write_register(&i2cTransaction, &tmpSettings, initData);

    //assigning critical temp
    Transaction_t curTransaction = {2,0,MSP9898_SlaveAddress,TMP_CRITICALBOUNDARY_REG};
    //I2C_write_register(&i2cTransaction, &tmpSettings, initData + 2);

    //assigning resolution bits
    Transaction_t curTransaction = {1,0,MSP9898_SlaveAddress,RESOLUTION_REG}; //only
    //I2C_write_register(&i2cTransaction, &tmpSettings, initData + 4);

    Enable_Interrupts();

}



//initializing Pin 4 on Port C
/*
 * @brief: On rising edge, when Tcur > Tcrit, power is shut off. Power turned
 *         back on when Tcur = Tcrit - hystersis.
 */
uint8_t RisingEdge = 1;
//do I need to add this? I can't find it in msp header?
void GPIOPORTC_Handler(void){
     //debounce input... its not a sw? so might not need it
     if(GPIOC->RIS & 0x10){
         if(RisingEdge){
            //Power_Off()
             RisingEdge = 0;
         }
         else{
            //Power_On()
            RisingEdge = 1;
         }
     }

 }



 /*
 * @brief: Obtains current temperature value(negative or positive) in decimal with .0625 (°C) resolution.
 *         When temperatures are in range of -40 and 100 (°C), there is .25 to .5 (°C) accuracy.
 *
 * @param: none
 *
 * @return: float containing decimal value of current temperature
 */
//outputs temperature values with .0625 (°C) resolution
//.25 to .5 (°C) accuracy for range of values between -40 and 100 (°C)
float MCP9808_GetTemp(void){
   uint8_t tempBuf[2]; //stores MSB for idx0, LSB for idx1 in 2s complement
   float curTemp;
   Transaction_t curTransaction = {0,2,MSP9898_SlaveAddress,CURRENT_TEMP_REG};
   //I2C_Setting_t tempSettings = {}; ???
   I2C_read_register(curTransaction,tempSettings,tmpBuf);
   tmpBuf[0] = tmpBuf[0] & 0x1F; //clearing out bits 15-13 bc need them cleared for calculation below
   if(tmpBuf[0] & 0x10){ //tmp < 0
      tmpBuf[1] &= ~0x10; //sign bit = 0;
      curTemp = -1 * ((tmpBuf[1] << 4) + (tmpBuf[0] / 16)); //TA = -1*( (UpperByte * 16) + (LowerByte/16)) (by nature of how reg stores data in 2s comp)
   }
   else{
       curTemp =  (tmpBuf[1] << 4) + (tmpBuf[0] / 16); //TA = UpperByte * 16 + LowerByte/16
   }

   return curTemp;
}






//TODO: all of this goes in seperate alarm module!!!


//Testing Q's
//call I2C Init func inside mcpdriver init(see comment in func)? feel like this is initalized before this tho in like module that calls
//i2c init and then my sensor's init etc.
// neg temp readings might be off(but u shldnt even read neg temp this tbh so i wldnt stress it)
// PORTC handler name is correct or naw? is ur logic for rising edge and falling edge working?
//debounce input... its not a sw? so might not need it





