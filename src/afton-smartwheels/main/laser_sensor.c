#include "laser_sensor.h"
// #include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "driver/gpio.h"
#include "VL53L4CD_api.h"
#include "platform.h"
#include "esp_log.h"

static const char *TAG = "LASER_SENSOR";

esp_err_t laser_sensor_init(VL53L4CD_DEV dev) {
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init(dev));
    sensor_power_cycle();

    dev->I2cDevAddr = VL53L4CD_I2C_ADDRESS; // Ensure this is the correct I2C address

    uint16_t sensor_id;
    ESP_LOGI(TAG, "VL53L4CD - tryGetSensorId");
    int status = VL53L4CD_GetSensorId(dev, &sensor_id);
    if (status != 0) {
        ESP_LOGE(TAG, "Failed to get sensor ID, status: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sensor ID: 0x%X", sensor_id);
    return ESP_OK;
}

esp_err_t laser_sensor_start_ranging(VL53L4CD_DEV dev) {
    int status = VL53L4CD_StartRanging(dev);
    if (status != 0) {
        ESP_LOGE(TAG, "Failed to start ranging, status: %d", status);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t laser_sensor_get_distance(VL53L4CD_DEV dev, uint16_t *distance) {
    VL53L4CD_ResultsData_t results;
    int status = VL53L4CD_GetDistance(dev, &results);
    if (status != 0) {
        ESP_LOGE(TAG, "Failed to get distance, status: %d", status);
        return ESP_FAIL;
    }
    *distance = results.distance_mm;
    return ESP_OK;
}

esp_err_t laser_sensor_stop_ranging(VL53L4CD_DEV dev) {
    int status = VL53L4CD_StopRanging(dev);
    if (status != 0) {
        ESP_LOGE(TAG, "Failed to stop ranging, status: %d", status);
        return ESP_FAIL;
    }
    return ESP_OK;
}