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

static const char *TAG = "WS_SERVER";


// // Function to process and crop the image to 640x230
// void process_image(uint16_t width, uint16_t height, pixformat_t format, uint8_t *buf, size_t len)
// {
//     // New height after cropping
//     uint16_t new_height = 230;
//     uint32_t new_len = width * new_height * 3; // 3 bytes per pixel for RGB

//     // Allocate memory for the cropped image
//     uint8_t *new_buf = (uint8_t *)malloc(new_len);
//     if (!new_buf) {
//         ESP_LOGE(TAG, "Failed to allocate memory for the cropped image");
//         return;
//     }

//     // Copy the relevant part of the image to the new buffer
//     uint32_t offset = width * (height - new_height) * 3; // Start from the bottom part of the image
//     memcpy(new_buf, buf + offset, new_len);

//     // Free the original buffer if necessary
//     // free(buf);

//     // Point the buffer to the new cropped image
//     buf = new_buf;
//     len = new_len;

//     // Log the new image dimensions and length
//     ESP_LOGI(TAG, "Cropped image width=%d, height=%d, len=%d", width, new_height, len);
// }


esp_err_t camera_capture(char * FileName, size_t *pictureSize)
{
	//clear internal queue
	//for(int i=0;i<2;i++) {
	for(int i=0;i<1;i++) {
		camera_fb_t * fb = esp_camera_fb_get();
		ESP_LOGI(TAG, "fb->len=%d", fb->len);
		ESP_LOGI(TAG, "fb->height=%d", fb->height);
		ESP_LOGI(TAG, "fb->width=%d", fb->width);
		esp_camera_fb_return(fb);
	}

	//acquire a frame
	camera_fb_t * fb = esp_camera_fb_get();
	if (!fb) {
		ESP_LOGE(TAG, "Camera Capture Failed");
		return ESP_FAIL;
	}

	//replace this with your own function
	//process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);
	FILE* f = fopen(FileName, "wb");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for writing");
		return ESP_FAIL; 
	}
	fwrite(fb->buf, fb->len, 1, f);
	ESP_LOGI(TAG, "fb->len=%d", fb->len);
	*pictureSize = (size_t)fb->len;
	fclose(f);
	
	//return the frame buffer back to the driver for reuse
	esp_camera_fb_return(fb);

	return ESP_OK;
}


// // Function to capture an image and save it to a specified path
// esp_err_t capture_image2(char *imageFileName, size_t imageFileNameSize) {
//     esp_err_t ret;
//     strcpy(imageFileName, "/spiffs/capture.jpeg");

//     // Delete local file if it exists
//     struct stat statBuf;
//     if (stat(imageFileName, &statBuf) == 0) {
//         unlink(imageFileName);
//         ESP_LOGI(TAG, "Delete Local file");
//     }

// #if CONFIG_ENABLE_FLASH
//     // Flash Light ON
//     gpio_set_level(CONFIG_GPIO_FLASH, 1);
// #endif

//     // Save Picture to Local file
//     int retryCounter = 0;
//     while (1) {
//         size_t pictureSize;
//         ret = camera_capture(imageFileName, &pictureSize);
//         ESP_LOGI(TAG, "camera_capture=%d", ret);
//         ESP_LOGI(TAG, "pictureSize=%d", pictureSize);
//         if (ret != ESP_OK) continue;
//         if (stat(imageFileName, &statBuf) == 0) {
//             ESP_LOGI(TAG, "st_size=%d", (int)statBuf.st_size);
//             if (statBuf.st_size == pictureSize) break;
//             retryCounter++;
//             ESP_LOGI(TAG, "Retry capture %d", retryCounter);
//             if (retryCounter > 10) {
//                 ESP_LOGE(TAG, "Retry over for capture");
//                 break;
//             }
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     }

// #if CONFIG_ENABLE_FLASH
//     // Flash Light OFF
//     gpio_set_level(CONFIG_GPIO_FLASH, 0);
// #endif

//     // Get Image size
//     if (stat(imageFileName, &statBuf) != 0) {
//         ESP_LOGE(TAG, "[%s] not found", imageFileName);
//         return ESP_FAIL;
//     }
//     ESP_LOGI(TAG, "%s st.st_size=%ld", imageFileName, statBuf.st_size);

//     return ESP_OK;
// }


// Calculate the size after conversion to base64
// http://akabanessa.blog73.fc2.com/blog-entry-83.html
int32_t calcBase64EncodedSize(int origDataSize)
{
	// Number of blocks in 6-bit units (rounded up in 6-bit units)
	int32_t numBlocks6 = ((origDataSize * 8) + 5) / 6;
	// Number of blocks in units of 4 characters (rounded up in units of 4 characters)
	int32_t numBlocks4 = (numBlocks6 + 3) / 4;
	// Number of characters without line breaks
	int32_t numNetChars = numBlocks4 * 4;
	// Size considering line breaks every 76 characters (line breaks are "\ r \ n")
	//return numNetChars + ((numNetChars / 76) * 2);
	return numNetChars;
}

esp_err_t Image2Base64(char * imageFileName, size_t base64_buffer_len, uint8_t * base64_buffer)
{
	struct stat st;
	if (stat(imageFileName, &st) != 0) {
		ESP_LOGE(TAG, "[%s] not found", imageFileName);
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "%s st.st_size=%ld", imageFileName, st.st_size);

	// Allocate image memory
	unsigned char*	image_buffer = NULL;
	size_t image_buffer_len = st.st_size;
	image_buffer = malloc(image_buffer_len);
	if (image_buffer == NULL) {
		ESP_LOGE(TAG, "malloc fail. image_buffer_len %d", image_buffer_len);
		return ESP_FAIL;
	}

	// Read image file
	FILE * fp_image = fopen(imageFileName,"rb");
	if (fp_image == NULL) {
		ESP_LOGE(TAG, "[%s] fopen fail.", imageFileName);
		free(image_buffer);
		return ESP_FAIL;
	}
	for (int i=0;i<st.st_size;i++) {
		fread(&image_buffer[i], sizeof(char), 1, fp_image);
	}
	fclose(fp_image);

	// Convert from JPEG to BASE64
	size_t encord_len;
	esp_err_t ret = mbedtls_base64_encode(base64_buffer, base64_buffer_len, &encord_len, image_buffer, st.st_size);
	ESP_LOGI(TAG, "mbedtls_base64_encode=%d encord_len=%d", ret, encord_len);

	free(image_buffer);
	return ESP_OK;
}
