#ifndef LASER_SENSOR_H
#define LASER_SENSOR_H

#include "esp_err.h"

// I2C Address of the VL53L4CD
#define VL53L4CD_I2C_ADDRESS 0x52

// I2C Port and Pins
#define I2C_MASTER_SCL_IO 41  // Set the correct GPIO number
#define I2C_MASTER_SDA_IO 40  // Set the correct GPIO number
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000  // Fast mode

// XSHUT Pin
#define XSHUT_PIN 23  // Set the correct GPIO number

// Function Prototypes
esp_err_t laser_sensor_init(void);
esp_err_t laser_sensor_start_ranging(void);
esp_err_t laser_sensor_stop_ranging(void);
esp_err_t laser_sensor_get_distance(uint16_t *distance);

#endif // LASER_SENSOR_H
