#include "laser_sensor.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "VL53L4CD_api.h"  // Include the VL53L4CD API header
#include "platform.h"

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
    gpio_pad_select_gpio(XSHUT_PIN);
    gpio_set_direction(XSHUT_PIN, GPIO_MODE_OUTPUT);

    // Power cycle the sensor
    gpio_set_level(XSHUT_PIN, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(XSHUT_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Initialize the sensor
    VL53L4CD_ULD_Init();
    VL53L4CD_ULD_WaitDeviceBooted(VL53L4CD_I2C_ADDRESS);
    VL53L4CD_ULD_DataInit(VL53L4CD_I2C_ADDRESS);

    return ESP_OK;
}

// Start Ranging
esp_err_t laser_sensor_start_ranging(void) {
    VL53L4CD_ULD_StartMeasurement(VL53L4CD_I2C_ADDRESS);
    return ESP_OK;
}

// Stop Ranging
esp_err_t laser_sensor_stop_ranging(void) {
    VL53L4CD_ULD_StopMeasurement(VL53L4CD_I2C_ADDRESS);
    return ESP_OK;
}

// Get Distance
esp_err_t laser_sensor_get_distance(uint16_t *distance) {
    VL53L4CD_ResultsData_t results;
    VL53L4CD_ULD_GetMeasurementDataReady(VL53L4CD_I2C_ADDRESS);
    VL53L4CD_ULD_GetRangingMeasurementData(VL53L4CD_I2C_ADDRESS, &results);
    *distance = results.distance_mm;
    VL53L4CD_ULD_ClearInterruptAndStartMeasurement(VL53L4CD_I2C_ADDRESS);
    return ESP_OK;
}
