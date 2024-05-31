#ifndef LASER_SENSOR_H
#define LASER_SENSOR_H

#include "esp_err.h"

#define I2C_MASTER_SCL_IO           41    // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO           40    // GPIO number for I2C master data
#define I2C_MASTER_NUM              I2C_NUM_0  // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ          100000     // I2C master clock frequency
#define XSHUT_PIN                   13    // GPIO number for XSHUT pin
#define VL53L4CD_I2C_ADDRESS        0x52  // Default I2C address for VL53L4CD

esp_err_t laser_sensor_init(void);
esp_err_t laser_sensor_start_ranging(void);
esp_err_t laser_sensor_stop_ranging(void);
esp_err_t laser_sensor_get_distance(uint16_t *distance);

#endif // LASER_SENSOR_H
