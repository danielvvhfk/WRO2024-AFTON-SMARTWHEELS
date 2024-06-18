/*
	Take a picture.

	This code is in the Public Domain (or CC0 licensed, at your option.)
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>
#include <mbedtls/base64.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"

#include "esp_camera.h"

#include "esp_http_server.h"
#include "camera_capture.h"

static const char *TAG = "WS_SERVER";


// Implementation of calcBase64EncodedSize
int32_t calcBase64EncodedSize(int size) {
    return ((size + 2) / 3) * 4;
}

// Implementation of Image2Base64
esp_err_t Image2Base64(const char *imageFileName, size_t base64BufferLen, uint8_t *base64Buffer) {
    // Add your implementation for converting an image to Base64 here
    return ESP_OK; // Return ESP_OK for the sake of this example
}

// Implementation of camera_capture
esp_err_t camera_capture(const char *imageFileName, size_t *pictureSize) {
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE(TAG, "Failed to capture image");
        return ESP_FAIL;
    }

    FILE *file = fopen(imageFileName, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        esp_camera_fb_return(pic);
        return ESP_FAIL;
    }

    fwrite(pic->buf, 1, pic->len, file);
    fclose(file);

    *pictureSize = pic->len;
    esp_camera_fb_return(pic);

    ESP_LOGI(TAG, "Image saved to %s, size: %d", imageFileName, *pictureSize);

    return ESP_OK;
}

esp_err_t get_image(char *imageFileName, size_t imageFileNameSize) {
    esp_err_t ret;
    strcpy(imageFileName, "/spiffs/capture.jpeg");

    // Delete local file if it exists
    struct stat statBuf;
    if (stat(imageFileName, &statBuf) == 0) {
        unlink(imageFileName);
        ESP_LOGI(TAG, "Delete Local file");
    }

#if CONFIG_ENABLE_FLASH
    // Flash Light ON
    gpio_set_level(CONFIG_GPIO_FLASH, 1);
#endif

    // Save Picture to Local file
    int retryCounter = 0;
    while (1) {
        size_t pictureSize;
        ret = camera_capture(imageFileName, &pictureSize);
        ESP_LOGI(TAG, "camera_capture=%d", ret);
        ESP_LOGI(TAG, "pictureSize=%d", pictureSize);
        if (ret != ESP_OK) continue;
        if (stat(imageFileName, &statBuf) == 0) {
            ESP_LOGI(TAG, "st_size=%d", (int)statBuf.st_size);
            if (statBuf.st_size == pictureSize) break;
            retryCounter++;
            ESP_LOGI(TAG, "Retry capture %d", retryCounter);
            if (retryCounter > 10) {
                ESP_LOGE(TAG, "Retry over for capture");
                break;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

#if CONFIG_ENABLE_FLASH
    // Flash Light OFF
    gpio_set_level(CONFIG_GPIO_FLASH, 0);
#endif

    // Get Image size
    if (stat(imageFileName, &statBuf) != 0) {
        ESP_LOGE(TAG, "[%s] not found", imageFileName);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "%s st.st_size=%ld", imageFileName, statBuf.st_size);

    // Get Base64 size
    int32_t base64Size = calcBase64EncodedSize(statBuf.st_size);
    ESP_LOGI(TAG, "base64Size=%" PRIi32, base64Size);

    // Allocate Base64 buffer
    uint8_t *base64_buffer = NULL;
    size_t base64_buffer_len = base64Size + 1;
    base64_buffer = calloc(1, base64_buffer_len);
    if (base64_buffer == NULL) {
        ESP_LOGE(TAG, "calloc fail. base64_buffer_len %d", base64_buffer_len);
        return ESP_FAIL;
    }
    memset(base64_buffer, 0, base64_buffer_len);

    // Convert from Image to Base64
    ret = Image2Base64(imageFileName, base64_buffer_len, base64_buffer);
    ESP_LOGI(TAG, "Image2Base64=%d", ret);
    if (ret != ESP_OK) {
        free(base64_buffer);
        return ret;
    }

    free(base64_buffer);

    return ESP_OK;
}