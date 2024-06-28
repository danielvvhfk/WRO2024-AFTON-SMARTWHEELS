#ifndef LASER_SENSOR_H
#define LASER_SENSOR_H

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "platform.h"

#define I2C0_MASTER_NUM           I2C_NUM_0 // Use I2C1 for the laser sensor
#define I2C0_MASTER_SDA_IO        GPIO_NUM_47 // Change to an available GPIO
#define I2C0_MASTER_SCL_IO        GPIO_NUM_48 // Change to an available GPIO
#define I2C0_MASTER_FREQ_HZ       100000      // Set frequency to 100kHz

#define XSHUT_PIN                   41    // GPIO number for XSHUT pin
#define VL53L4CD_I2C_ADDRESS        0x52  // Default I2C address for VL53L4CD
#define I2C0_MASTER_TX_BUF_DISABLE   0          // I2C master doesn't need buffer
#define I2C0_MASTER_RX_BUF_DISABLE   0          // I2C master doesn't need buffer


esp_err_t i2c_master_init();
void sensor_power_cycle();

esp_err_t laser_sensor_init(VL53L4CD_DEV dev);
esp_err_t laser_sensor_start_ranging(VL53L4CD_DEV dev);
esp_err_t laser_sensor_get_distance(VL53L4CD_DEV dev, uint16_t *distance);
esp_err_t laser_sensor_stop_ranging(VL53L4CD_DEV dev);


#endif // LASER_SENSOR_H
