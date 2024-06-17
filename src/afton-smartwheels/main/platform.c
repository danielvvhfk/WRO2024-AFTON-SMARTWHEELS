/*
 Copyright (c) 2021, STMicroelectronics - All Rights Reserved

 This file : part of VL53L4CD Ultra Lite Driver and : dual licensed, either
 'STMicroelectronics Proprietary license'
 or 'BSD 3-clause "New" or "Revised" License' , at your option.

*******************************************************************************

 'STMicroelectronics Proprietary license'

*******************************************************************************

 License terms: STMicroelectronics Proprietary in accordance with licensing
 terms at www.st.com/sla0081

 STMicroelectronics confidential
 Reproduction and Communication of this document : strictly prohibited unless
 specifically authorized in writing by STMicroelectronics.


*******************************************************************************

 Alternatively, VL53L4CD Ultra Lite Driver may be distributed under the terms of
 'BSD 3-clause "New" or "Revised" License', in which case the following
 provisions apply instead of the ones mentioned above :

*******************************************************************************

 License terms: BSD 3-clause "New" or "Revised" License.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
*/

#include "driver/i2c.h"
#include "esp_log.h"
#include "platform.h"
#include <freertos/FreeRTOS.h>

#define I2C_MASTER_NUM I2C_NUM_0   // I2C port number for master dev
#define I2C_MASTER_TX_BUF_DISABLE 0   // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0   // I2C master doesn't need buffer
#define I2C_MASTER_FREQ_HZ 100000     // I2C master clock frequency

#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "VL53L4CD_PLATFORM";


// uint8_t VL53L4CD_RdDWord(Dev_t dev, uint16_t RegisterAdress, uint32_t *value)
// {
// 	uint8_t status = 255;
	
// 	/* To be filled by customer. Return 0 if OK */
// 	/* Warning : For big endian platforms, fields 'RegisterAdress' and 'value' need to be swapped. */
	
// 	return status;
// }

// uint8_t VL53L4CD_RdWord(Dev_t dev, uint16_t RegisterAdress, uint16_t *value) {
//     uint8_t data[2];
//     uint8_t status = 0;

//     ESP_LOGI(TAG, "Reading I2C Word from dev: 0x%x, register: 0x%x", dev, RegisterAdress);

//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, dev << 1 | I2C_MASTER_WRITE, true);
//     i2c_master_write_byte(cmd, (RegisterAdress >> 8) & 0xFF, true); // MSB of RegisterAdress
//     i2c_master_write_byte(cmd, RegisterAdress & 0xFF, true); // LSB of RegisterAdress
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, dev << 1 | I2C_MASTER_READ, true);
//     i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
//     i2c_master_stop(cmd);
//     status = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);

//     if (status == ESP_OK) {
//         *value = (data[0] << 8) | data[1]; // Combine MSB and LSB
//         ESP_LOGI(TAG, "I2C Word read successful, value: 0x%x", *value);
//     } else {
//         ESP_LOGE(TAG, "Error in I2C read: %s", esp_err_to_name(status));
//         status = 255; // Custom error code
//     }

//     return status;
// }

// // Function to write a single byte to a given register
// uint8_t VL53L4CD_WrByte(Dev_t dev, uint16_t RegisterAddress, uint8_t value) {
//     uint8_t status = 0;

//     ESP_LOGI(TAG, "Writing byte to I2C dev: 0x%x, register: 0x%x, value: 0x%x", dev, RegisterAddress, value);

//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, dev << 1 | I2C_MASTER_WRITE, true);
//     i2c_master_write_byte(cmd, (RegisterAddress >> 8) & 0xFF, true); // MSB of RegisterAddress
//     i2c_master_write_byte(cmd, RegisterAddress & 0xFF, true); // LSB of RegisterAddress
//     i2c_master_write_byte(cmd, value, true);
//     i2c_master_stop(cmd);
//     status = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);

//     if (status == ESP_OK) {
//         ESP_LOGI(TAG, "I2C Write successful");
//     } else {
//         ESP_LOGE(TAG, "Error in I2C write: %s", esp_err_to_name(status));
//         status = 255; // Custom error code
//     }

//     return status;
// }

// // Function to read a single byte from a given register
// uint8_t VL53L4CD_RdByte(Dev_t dev, uint16_t RegisterAddress, uint8_t *value) {
//     uint8_t status = 0;

//     // ESP_LOGI(TAG, "Reading byte from I2C dev: 0x%x, register: 0x%x", dev, RegisterAddress);

//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, dev << 1 | I2C_MASTER_WRITE, true);
//     i2c_master_write_byte(cmd, (RegisterAddress >> 8) & 0xFF, true); // MSB of RegisterAddress
//     i2c_master_write_byte(cmd, RegisterAddress & 0xFF, true); // LSB of RegisterAddress
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, dev << 1 | I2C_MASTER_READ, true);
//     i2c_master_read_byte(cmd, value, I2C_MASTER_NACK);
//     i2c_master_stop(cmd);
//     status = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);

//     if (status == ESP_OK) {
//         ESP_LOGI(TAG, "I2C Read successful, value: 0x%x", *value);
//     } else {
//         // ESP_LOGE(TAG, "Error in I2C read: %s", esp_err_to_name(status));
//         status = 255; // Custom error code
//     }

//     return status;
// }


// uint8_t VL53L4CD_WrWord(Dev_t dev, uint16_t RegisterAdress, uint16_t value) {
//     uint8_t status = 0;
//     uint8_t data[2] = { (value >> 8) & 0xFF, value & 0xFF };

//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, dev << 1 | I2C_MASTER_WRITE, true);
//     i2c_master_write_byte(cmd, (RegisterAdress >> 8) & 0xFF, true); // MSB of RegisterAdress
//     i2c_master_write_byte(cmd, RegisterAdress & 0xFF, true); // LSB of RegisterAdress
//     i2c_master_write(cmd, data, 2, true);
//     i2c_master_stop(cmd);
//     status = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);

//     if (status != ESP_OK) {
//         ESP_LOGE(TAG, "Error in I2C write: %s", esp_err_to_name(status));
//         status = 255; // Custom error code
//     }

//     return status;
// }

// uint8_t VL53L4CD_WrDWord(Dev_t dev, uint16_t RegisterAdress, uint32_t value)
// {
// 	uint8_t status = 255;

// 	/* To be filled by customer. Return 0 if OK */
// 	/* Warning : For big endian platforms, fields 'RegisterAdress' and 'value' need to be swapped. */
	
// 	return status;
// }

// uint8_t WaitMs(Dev_t dev, uint32_t TimeMs)
// {
// 	/* To be filled by customer */
// 	vTaskDelay(pdMS_TO_TICKS(TimeMs));
// 	return 0; // Return 0 to indicate success
// }



uint8_t VL53L4CD_WrByte(Dev_t dev, uint16_t RegisterAdress, uint8_t value) {
    uint8_t status = 255;
    uint8_t buffer[3];
    buffer[0] = (uint8_t)(RegisterAdress >> 8); // MSB of register address
    buffer[1] = (uint8_t)(RegisterAdress & 0xFF); // LSB of register address
    buffer[2] = value;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, sizeof(buffer), true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        status = 0;
    } else {
        ESP_LOGE(TAG, "Error in I2C write: %s", esp_err_to_name(ret));
    }
    return status;
}

uint8_t VL53L4CD_RdByte(Dev_t dev, uint16_t RegisterAdress, uint8_t *value) {
    uint8_t status = 255;
    uint8_t reg_addr[2];
    reg_addr[0] = (uint8_t)(RegisterAdress >> 8); // MSB of register address
    reg_addr[1] = (uint8_t)(RegisterAdress & 0xFF); // LSB of register address

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, reg_addr, sizeof(reg_addr), true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, value, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        status = 0;
    } else {
        ESP_LOGE(TAG, "Error in I2C read: %s", esp_err_to_name(ret));
    }
    return status;
}

uint8_t VL53L4CD_RdWord(Dev_t dev, uint16_t RegisterAdress, uint16_t *value) {
    uint8_t status = 255;
    uint8_t reg_addr[2];
    uint8_t data[2];
    reg_addr[0] = (uint8_t)(RegisterAdress >> 8);
    reg_addr[1] = (uint8_t)(RegisterAdress & 0xFF);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, reg_addr, sizeof(reg_addr), true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, sizeof(data), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        *value = (data[0] << 8) | data[1];
        status = 0;
    } else {
        ESP_LOGE(TAG, "Error in I2C read: %s", esp_err_to_name(ret));
    }
    return status;
}


uint8_t VL53L4CD_WrWord(Dev_t dev, uint16_t RegisterAdress, uint16_t value) {
    uint8_t status = 255;
    uint8_t buffer[4];
    buffer[0] = (uint8_t)(RegisterAdress >> 8); // MSB of register address
    buffer[1] = (uint8_t)(RegisterAdress & 0xFF); // LSB of register address
    buffer[2] = (uint8_t)(value >> 8); // MSB of value
    buffer[3] = (uint8_t)(value & 0xFF); // LSB of value

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, sizeof(buffer), true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        status = 0;
    } else {
        ESP_LOGE(TAG, "Error in I2C write: %s", esp_err_to_name(ret));
    }
    return status;
}


uint8_t VL53L4CD_WrDWord(Dev_t dev, uint16_t RegisterAdress, uint32_t value) {
    uint8_t status = 255;
    uint8_t buffer[6];
    buffer[0] = (uint8_t)(RegisterAdress >> 8); // MSB of register address
    buffer[1] = (uint8_t)(RegisterAdress & 0xFF); // LSB of register address
    buffer[2] = (uint8_t)(value >> 24); // MSB of value
    buffer[3] = (uint8_t)((value >> 16) & 0xFF);
    buffer[4] = (uint8_t)((value >> 8) & 0xFF);
    buffer[5] = (uint8_t)(value & 0xFF); // LSB of value

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, sizeof(buffer), true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        status = 0;
    } else {
        ESP_LOGE(TAG, "Error in I2C write: %s", esp_err_to_name(ret));
    }
    return status;
}


uint8_t VL53L4CD_RdDWord(Dev_t dev, uint16_t RegisterAdress, uint32_t *value) {
    uint8_t status = 255;
    uint8_t reg_addr[2];
    uint8_t data[4];
    reg_addr[0] = (uint8_t)(RegisterAdress >> 8);
    reg_addr[1] = (uint8_t)(RegisterAdress & 0xFF);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, reg_addr, sizeof(reg_addr), true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, sizeof(data), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        *value = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];
        status = 0;
    } else {
        ESP_LOGE(TAG, "Error in I2C read: %s", esp_err_to_name(ret));
    }
    return status;
}

uint8_t WaitMs(Dev_t dev, uint32_t TimeMs) {
    vTaskDelay(pdMS_TO_TICKS(TimeMs));
    return 0;
}
