

#include "driver/gpio.h"
#include "esp_log.h"


#include "laser_sensor.h"
#include "platform.h"
#include "VL53L4CD_api.h"


static const char *TAG = "LASER_SENSOR";
static VL53L4CD_Dev_t laser_sensor = {0};

esp_err_t laser_sensor_init(VL53L4CD_DEV dev) {
    // Initialize I2C
    ESP_LOGI(TAG, "Initialize I2C for sensor");
     esp_err_t ret = i2c_master_init(&laser_sensor);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C");
        return ret;
    }

    sensor_power_cycle();

    dev->I2cDevAddr = VL53L4CD_I2C_ADDRESS; // Ensure this is the correct I2C address

    // Get sensor ID
    uint16_t sensor_id = 0;
    ret = VL53L4CD_GetSensorId(&laser_sensor, &sensor_id);

    // Log the values of ret and sensor_id
    ESP_LOGI(TAG, "ret: 0x%02X, sensor_id: 0x%04X", ret, sensor_id);

    if (ret != ESP_OK || sensor_id != 0xEACC) {
        ESP_LOGE(TAG, "Failed to get sensor ID or wrong sensor ID");
        return ret;
    }

    ESP_LOGI(TAG, "Sensor ID: 0x%X", sensor_id);
    return ESP_OK;
}


void sensor_power_cycle() {
    ESP_LOGI(TAG, "Sensor power cycle");
    gpio_set_direction(XSHUT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(XSHUT_PIN, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Power down time
    gpio_set_level(XSHUT_PIN, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Power up time
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