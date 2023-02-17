/*
 * MPU6050.c
 *
 *  Last Modified on: Feb 13, 2023
 *      Author: Sourish Wawdhane
 */
#include <stdint.h>
#include <guadaloop/drivers/sensors/acceleration/MPU6050.h>


static int accel_divider = 0;
                               //+- 2g, 4g, 8g, 16g respectively
static int accel_divider_values[4] = {16384, 8192, 4096, 2048};

typedef enum sensor_register_t {gyroconfig, acceleroconfig,accel_x_15_8,accel_x_7_0,accel_y_15_8,accel_y_7_0,accel_z_15_8,accel_z_7_0} sensor_register;

static uint8_t get_register_number(sensor_register r){
    switch (r){
    case gyroconfig:
        return 0x1B;
    case acceleroconfig:
        return 0x1C;
    case accel_x_15_8: // higher bit
        return 0x3B;
    case accel_x_7_0:
        return 0x3C;

    case accel_y_15_8:
        return 0x3D;
    case accel_y_7_0:
        return 0x3E;

    case accel_z_15_8:
        return 0x3F;
    case accel_z_7_0:
        return 0x40;

    }
    return 0;
}

void get_register(sensor_register r, uint8_t* data_to_retrieve){
    uint8_t real_register_number = get_register_number(r);
    //*data_to_store =
    // TODO interface with I2C

}

void set_register(sensor_register r, uint8_t* data_to_set){
    uint8_t real_register_number = get_register_number(r);

    // TODO interface with I2C

}

/*
AFS_SEL Full Scale Range
0: ± 2g
1: ± 4g
2: ± 8g
3: ± 16g
 */

void config_accelerometer(int8_t full_scale_range){
    if (full_scale_range>3){return -1;}

    uint8_t FS_SEL = full_scale_range & 0x03;
    FS_SEL = FS_SEL << 3;
    uint8_t current_accelero_configuration;
    get_register(acceleroconfig, &current_accelero_configuration);
    // clear what's there
    current_accelero_configuration &= (~(0x03<<3));
    current_accelero_configuration |= FS_SEL;
    set_register(acceleroconfig, &current_accelero_configuration);
    accel_divider = accel_divider_values[full_scale_range];

}

// in the thousands
int16_t sample_accelerometer(sensor_register higher_register, sensor_register lower_register){
    uint8_t higher_byte;
    uint8_t lower_byte;

    get_register(higher_register, &higher_byte);
    get_register(lower_register, &lower_byte);

    int32_t total_sample = (higher_byte << 8) | lower_byte;
    total_sample *= fixed_point_value_multiplier;
    total_sample /= accel_divider;
    return (total_sample);

}

int16_t sample_x_acceleration(){
    return sample_accelerometer(accel_x_15_8, accel_x_7_0);
}

int16_t sample_y_acceleration(){
    return sample_accelerometer(accel_y_15_8, accel_y_7_0);
}

int16_t sample_z_acceleration(){
    return sample_accelerometer(accel_z_15_8, accel_z_7_0);
}


/* The following is a description of the pins that are going to be used for the MPU6050
 * Pin 8: Digital I/O supply voltage
 * Pin 9: LSB AD0 Slave Address
 * Pin 23: SCL I2C serial clock
 * Pin 24: I2C serial data
 * Clock speed: 400kHz
 */

/*
 * Pertinent Registers:
 * GYROCONFIG Page 14
 * ACCELEROCONFIG Page 15
 * ACCELEROSAMPLE Page 29
 *
 */

/*
 * How to write to a register:
 * Send the start condition
 * Send the I2C address with the write bit
 *
 * Send the register address
 * Send the register data
 * Keep sending data until conclusion
 * Send the stop condition
 *
 * Page 35 in https://product.tdk.com/system/files/dam/doc/product/sensor/mortion-inertial/imu/data_sheet/mpu-6000-datasheet1.pdf
 */

/*
 * How to read a register:
 * Send the start condition
 * Send the I2C address with the write bit
 * Send the register address to read
 *
 * Send the start signal
 * Send the I2C address with read bit
 * The MPU6050 sends the data
 * Ack from us
 * Send data
 * repeat until done
 * nack
 *
 */


/*
   int full_scale_range:
   FS_SEL Full Scale Range
   0: ± 250 °/s
   1: ± 500 °/s
   2: ± 1000 °/s
   3: ± 2000 °/s
*/

/* Currently unused at the moment:
int config_gyroscope(uint8_t full_scale_range){
    if (full_scale_range>3){return -1;}

    uint8_t FS_SEL = full_scale_range & 0x03;
    FS_SEL = FS_SEL << 3;
    uint8_t current_gyro_configuration;
    get_register(gyroconfig, &current_gyro_configuration);
    // clear what's there
    current_gyro_configuration &= (~(0x03<<3));
    current_gyro_configuration |= FS_SEL;
    set_register(gyroconfig, &current_gyro_configuration);
    return 1;
}

*/
