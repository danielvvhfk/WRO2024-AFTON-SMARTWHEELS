<<<<<<< HEAD
#include "laser_sensor.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
=======


>>>>>>> 422aa653227b7e71521a406d48b5df2887d1e610
#include "driver/gpio.h"
#include "esp_log.h"


#include "laser_sensor.h"
#include "platform.h"
<<<<<<< HEAD
#include "esp_log.h"
=======
#include "VL53L4CD_api.h"
>>>>>>> 422aa653227b7e71521a406d48b5df2887d1e610


<<<<<<< HEAD
// I2C Master Initialization
static esp_err_t i2c_master_init(void) {
    ESP_LOGI(TAG, "Initializing I2C...");
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C0_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C0_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C0_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C0_MASTER_NUM, &conf);
    ESP_LOGI(TAG, "I2C driver install");
    esp_err_t ret = i2c_driver_install(I2C0_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install error: %s", esp_err_to_name(ret));
    }
    return ret;
}

// Laser Sensor Initialization
esp_err_t laser_sensor_init(void) {
    esp_err_t ret;
    VL53L4CD_Error status;
    uint16_t sensor_id;
    VL53L4CD_Dev_t dev = { .I2cDevAddr = VL53L4CD_I2C_ADDRESS };  // Initialize device structure

    ESP_LOGI(TAG, "Initializing I2C...");
    ret = i2c_master_init();
=======
static const char *TAG = "LASER_SENSOR";
static VL53L4CD_Dev_t laser_sensor = {0};

esp_err_t laser_sensor_init(VL53L4CD_DEV dev) {
    // Initialize I2C
    ESP_LOGI(TAG, "Initialize I2C for sensor");
     esp_err_t ret = i2c_master_init(&laser_sensor);
>>>>>>> 422aa653227b7e71521a406d48b5df2887d1e610
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C");
        return ret;
    }

<<<<<<< HEAD
    ESP_LOGI(TAG, "Initialize the XSHUT pin");
    gpio_reset_pin(XSHUT_PIN);
    ret = gpio_set_direction(XSHUT_PIN, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO direction: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Power cycle the sensor");
    gpio_set_level(XSHUT_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(XSHUT_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "VL53L4CD- tryGetSensorId");

    status = VL53L4CD_GetSensorId(&dev, &sensor_id);
    if (status || (sensor_id != 0xEBAA)) {
        ESP_LOGE(TAG, "VL53L4CD not detected at requested address with ret status: %d", status);
        ESP_LOGE(TAG, "VL53L4CD sensorid: %d", sensor_id);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Initialize the sensor VL53L4CD");
    status = VL53L4CD_SensorInit(&dev);
    if (status != VL53L4CD_ERROR_NONE) {
        ESP_LOGE(TAG, "Sensor init failed with status: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "VL53L4CD ULD ready!");
    return ESP_OK;
}


// Start Ranging
esp_err_t laser_sensor_start_ranging(void) {
    VL53L4CD_Error status;
    status = VL53L4CD_StartRanging(VL53L4CD_I2C_ADDRESS);
    if (status != VL53L4CD_ERROR_NONE) {
        ESP_LOGE(TAG, "Start ranging failed with status: %d", status);
=======
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
>>>>>>> 422aa653227b7e71521a406d48b5df2887d1e610
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