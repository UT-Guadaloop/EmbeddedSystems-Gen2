/*
 * MPU6050.h
 *
 *  Created on: Dec 31, 2022
 *      Author: dario
 */

#ifndef MPU6050_H_
#define MPU6050_H_

#define fixed_point_value_multiplier 1000


/*
    config_accelerometer is a function which sets configuration settings for the accelerometer,
    including the ranges of values to measure.

    The MPU6050 in particular supports different levels of precision. For example, the 16g setting
    supports a greater range of g values, in return for each measurement being not as precise as
    it would have been for 2g. For this reason, please select the lowest range that is necessary.

    Pass in values according to the following:

    AFS_SEL Full Scale Range
    0: ± 2g
    1: ± 4g
    2: ± 8g
    3: ± 16g

 */
void config_accelerometer(int8_t full_scale_range);


/*

    These functions return a 16 bit, fixed point decimal number which represents the acceleration in g's.
    If you would like to change at which point it's fixed, you may do so by changing the
    fixed_point_value_multiplier macro above.

    Please ensure that you have called config_accelerometer before sampling values. It will not
    work otherwise. After calling config_accelerometer, you may sample acceleration in all 3
    dimensions using the following three functions.


 */

int16_t sample_x_acceleration();
int16_t sample_y_acceleration();
int16_t sample_z_acceleration();



#endif /* SRC_SENSORS_ACCELERATION_HEADERS_MPU6050_H_ */
