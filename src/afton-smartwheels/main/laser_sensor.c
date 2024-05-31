#include "laser_sensor.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "VL53L4CD_api.h"
#include "platform.h"
#include "esp_log.h"  // Add this include for ESP_LOGE

static const char *TAG = "VL53L4CD";

// I2C Master Initialization
static esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Laser Sensor Initialization
esp_err_t laser_sensor_init(void) {
    i2c_master_init();

    // Initialize the XSHUT pin
    gpio_reset_pin(XSHUT_PIN);  // Reset the pin before setting direction
    esp_err_t ret = gpio_set_direction(XSHUT_PIN, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO direction: %s", esp_err_to_name(ret));
        return ret;
    }

    // Power cycle the sensor
    gpio_set_level(XSHUT_PIN, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(XSHUT_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Initialize the sensor
    VL53L4CD_Error status;
    status = VL53L4CD_SensorInit(VL53L4CD_I2C_ADDRESS);
    if (status != VL53L4CD_ERROR_NONE) {
        ESP_LOGE(TAG, "Sensor init failed with status: %d", status);
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Start Ranging
esp_err_t laser_sensor_start_ranging(void) {
    VL53L4CD_Error status;
    status = VL53L4CD_StartRanging(VL53L4CD_I2C_ADDRESS);
    if (status != VL53L4CD_ERROR_NONE) {
        ESP_LOGE(TAG, "Start ranging failed with status: %d", status);
        return ESP_FAIL;
    }
    return ESP_OK;
}

// Stop Ranging
esp_err_t laser_sensor_stop_ranging(void) {
    VL53L4CD_Error status;
    status = VL53L4CD_StopRanging(VL53L4CD_I2C_ADDRESS);
    if (status != VL53L4CD_ERROR_NONE) {
        ESP_LOGE(TAG, "Stop ranging failed with status: %d", status);
        return ESP_FAIL;
    }
    return ESP_OK;
}

// Get Distance
esp_err_t laser_sensor_get_distance(uint16_t *distance) {
    VL53L4CD_ResultsData_t results;
    VL53L4CD_Error status;
    status = VL53L4CD_CheckForDataReady(VL53L4CD_I2C_ADDRESS, &results.range_status);
    if (status != VL53L4CD_ERROR_NONE || results.range_status != 0) {
        ESP_LOGE(TAG, "Data not ready or error: %d", status);
        return ESP_FAIL;
    }
    status = VL53L4CD_GetResult(VL53L4CD_I2C_ADDRESS, &results);
    if (status != VL53L4CD_ERROR_NONE) {
        ESP_LOGE(TAG, "Get result failed with status: %d", status);
        return ESP_FAIL;
    }
    *distance = results.distance_mm;
    VL53L4CD_ClearInterrupt(VL53L4CD_I2C_ADDRESS);
    return ESP_OK;
}
