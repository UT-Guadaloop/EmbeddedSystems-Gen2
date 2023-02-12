/*
 * mcp9808.c
 *
 *@brief   high level overview
 *
 *Note: Must call I2C_Init() before u call any read/write to
 *      sensor functions in higher level, more controller like files
 * Created on: Dec 31, 2022
 *      Author: TY Brinker, Dario
 */

#include <guadaloop/drivers/sensors/temperature/mcp9808.h>

//TODO

;
void MCP9808_Init(void){

    //disabeling sensor power consuming activites
    i2c_start();//send start command
    i2c_write(Ad); //

}


void MCP9808_SetTempAlertBoundaries{
    //keeping these 2 seperate functions bc when i write, i dont need to always
    //generate a start condition
    I2C0_Start()
    I2C0_Write() // write
}


I2C0_Start(){

}

I2C0_Read()
