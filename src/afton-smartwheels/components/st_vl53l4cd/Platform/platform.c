#include "platform.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"

#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "platform";

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO        GPIO_NUM_47 // Change to an available GPIO
#define I2C_MASTER_SCL_IO        GPIO_NUM_48 // Change to an available GPIO
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_TIMEOUT_MS 1000

// // I2C master initialization
// static esp_err_t i2c_master_init(void) {
//     i2c_config_t conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = I2C_MASTER_SDA_IO,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_io_num = I2C_MASTER_SCL_IO,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = I2C_MASTER_FREQ_HZ,
//     };
//     i2c_param_config(I2C_MASTER_NUM, &conf);
//     return i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_TX_BUF_DISABLE, I2C_MASTER_RX_BUF_DISABLE, 0);
// }

// Read a dword from the device
int8_t VL53L4CD_RdDWord(VL53L4CD_Dev_t *pdev, uint16_t index, uint32_t *pdata) {
    uint8_t indxBffr[2];
    indxBffr[0] = (index >> 8) & 0xFF;
    indxBffr[1] = index & 0xFF;
    uint8_t valueBffr[4];
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, pdev->I2cDevAddr, indxBffr, 2, valueBffr, 4, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Read failed: %s", esp_err_to_name(err));
        return -1;
    }
    *pdata = (valueBffr[0] << 24) | (valueBffr[1] << 16) | (valueBffr[2] << 8) | valueBffr[3];
    return 0;
}

// Write a byte to the device
int8_t VL53L4CD_WrByte(VL53L4CD_Dev_t *pdev, uint16_t index, uint8_t data) {
    uint8_t wrBffr[3];
    wrBffr[0] = (index >> 8) & 0xFF;
    wrBffr[1] = index & 0xFF;
    wrBffr[2] = data;
    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, pdev->I2cDevAddr, wrBffr, 3, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Write failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

uint8_t VL53L4CD_RdWord(VL53L4CD_Dev_t *pdev, uint16_t index, uint16_t *pdata) {
    if (pdev == NULL || pdata == NULL) {
        ESP_LOGE(TAG, "Invalid argument: pdev or pdata is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t indxBffr[2];
    indxBffr[0] = (index >> 8) & 0xFF;
    indxBffr[1] = index & 0xFF;
    uint8_t valueBffr[2];
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, pdev->I2cDevAddr, indxBffr, 2, valueBffr, 2, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Read failed: %s", esp_err_to_name(err));
        return -1;
    }
    *pdata = (valueBffr[0] << 8) | valueBffr[1];
    return 0;
}


// Read a byte from the device
uint8_t VL53L4CD_RdByte(VL53L4CD_Dev_t *pdev, uint16_t index, uint8_t *pdata) {
    uint8_t indxBffr[2];
    indxBffr[0] = (index >> 8) & 0xFF;
    indxBffr[1] = index & 0xFF;
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, pdev->I2cDevAddr, indxBffr, 2, pdata, 1, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Read failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

// Write a word to the device
uint8_t VL53L4CD_WrWord(VL53L4CD_Dev_t *pdev, uint16_t index, uint16_t data) {
    uint8_t wrBffr[4];
    wrBffr[0] = (index >> 8) & 0xFF;
    wrBffr[1] = index & 0xFF;
    wrBffr[2] = (data >> 8) & 0xFF;
    wrBffr[3] = data & 0xFF;
    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, pdev->I2cDevAddr, wrBffr, 4, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Write failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}

// Write a dword to the device
uint8_t VL53L4CD_WrDWord(VL53L4CD_Dev_t *pdev, uint16_t index, uint32_t data) {
    uint8_t wrBffr[6];
    wrBffr[0] = (index >> 8) & 0xFF;
    wrBffr[1] = index & 0xFF;
    wrBffr[2] = (data >> 24) & 0xFF;
    wrBffr[3] = (data >> 16) & 0xFF;
    wrBffr[4] = (data >> 8) & 0xFF;
    wrBffr[5] = data & 0xFF;
    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, pdev->I2cDevAddr, wrBffr, 6, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C Write failed: %s", esp_err_to_name(err));
        return -1;
    }
    return 0;
}


uint8_t WaitMs(Dev_t dev, uint32_t TimeMs)
{
    // dependent on sdkConfig (menuConfig) for portTICK_PERIOD_MS
    vTaskDelay(TimeMs / portTICK_PERIOD_MS);
	return 0;
}
