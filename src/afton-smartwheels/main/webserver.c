/*
	Take a picture and Publish it via Web Socket.

	This code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.

	I based from here:
	https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/ws_echo_server
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
#include "image_sender.h" // Adjust this include to match your actual file names


static const char *TAG = "WS_SERVER";


static esp_err_t get_handler(httpd_req_t *req)
{
	if (req->method == HTTP_GET) {
		ESP_LOGI(TAG, "Handshake done, the new connection was opened");
		return ESP_OK;
	}
	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	/* Set max_len = 0 to get the frame len */
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
		return ret;
	}
	ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
	if (ws_pkt.len) {
		/* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
		buf = calloc(1, ws_pkt.len + 1);
		if (buf == NULL) {
			ESP_LOGE(TAG, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;
		
		/* Set max_len = ws_pkt.len to get the frame payload */
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
		ESP_LOGI(TAG, "Got packet with message: [%.*s]", ws_pkt.len, ws_pkt.payload);
		// free(buf);
	}
	// Immediately after receiving the WebSocket message:
	char* receivedMsg = strndup((char*)buf, ws_pkt.len);


	ESP_LOGI(TAG, "Packet final: %d", ws_pkt.final);
	ESP_LOGI(TAG, "Packet fragmented: %d", ws_pkt.fragmented);
	ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);

	 // Log the received message
    ESP_LOGI(TAG, "Got packet with message2: [%.*s]", ws_pkt.len,receivedMsg);

    if (strncmp(receivedMsg, "capture", ws_pkt.len) == 0 || strncmp(receivedMsg, "reportapi", ws_pkt.len) == 0) {
    	char imageFileName[256];
		//strcpy(imageFileName, "/spiffs/esp32.jpeg");
		strcpy(imageFileName, "/spiffs/capture.jpeg");

		// Delete local file
		struct stat statBuf;
		if (stat(imageFileName, &statBuf) == 0) {
			// Delete it if it exists
			unlink(imageFileName);
			ESP_LOGI(TAG, "Delete Local file");
		}

#if CONFIG_ENABLE_FLASH
		// Flash Light ON
		gpio_set_level(CONFIG_GPIO_FLASH, 1);
#endif

		// Save Picture to Local file
		int retryCounter = 0;
		while(1) {
			size_t pictureSize;
			ret = camera_capture(imageFileName, &pictureSize);
			ESP_LOGI(TAG, "camera_capture=%d",ret);
			ESP_LOGI(TAG, "pictureSize=%d",pictureSize);
			if (ret != ESP_OK) continue;
			if (stat(imageFileName, &statBuf) == 0) {
				ESP_LOGI(TAG, "st_size=%d", (int)statBuf.st_size);
				if (statBuf.st_size == pictureSize) break;
				retryCounter++;
				ESP_LOGI(TAG, "Retry capture %d",retryCounter);
				if (retryCounter > 10) {
					ESP_LOGE(TAG, "Retry over for capture");
					break;
				}
				vTaskDelay(1000);
			}
		} // end while

#if CONFIG_ENABLE_FLASH
		// Flash Light OFF
		gpio_set_level(CONFIG_GPIO_FLASH, 0);
#endif

		// Get Image size
		struct stat st;
		if (stat(imageFileName, &st) != 0) {
			ESP_LOGE(TAG, "[%s] not found", imageFileName);
			return ESP_FAIL;
		}
		ESP_LOGI(TAG, "%s st.st_size=%ld", imageFileName, st.st_size);

		// Get Base64 size
		int32_t base64Size = calcBase64EncodedSize(st.st_size);
		ESP_LOGI(TAG, "base64Size=%"PRIi32, base64Size);

		// Allocate Base64 buffer
		// You have to use calloc. It doesn't work with malloc.
		uint8_t *base64_buffer = NULL;
		size_t base64_buffer_len = base64Size + 1;
		base64_buffer = calloc(1, base64_buffer_len);
		if (base64_buffer == NULL) {
			ESP_LOGE(TAG, "calloc fail. base64_buffer_len %d", base64_buffer_len);
			return ESP_FAIL;
		}
		memset(base64_buffer, 0, base64_buffer_len);

		// Convert from Image to Base64
		//ret = Image2Base64("/spiffs/esp32.jpeg", base64_buffer_len, base64_buffer);
		ret = Image2Base64(imageFileName, base64_buffer_len, base64_buffer);
		ESP_LOGI(TAG, "Image2Base64=%d", ret);
		if (ret != ESP_OK) {
			free(base64_buffer);
			return ret;
		}
			
		// Send by WebSocket
		ESP_LOGI(TAG, "Sendby WebSocket");
		ws_pkt.payload = base64_buffer;
		ws_pkt.len = base64Size;
		ret = httpd_ws_send_frame(req, &ws_pkt);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
		}
		free(base64_buffer);

		 ESP_LOGI(TAG, "Got packet with message3: [%.*s]", ws_pkt.len, receivedMsg);

        if (strncmp(receivedMsg, "reportapi", ws_pkt.len) == 0) {
			ESP_LOGI(TAG, "Send the image to the API");
            // Send the image to the API
            // The function send_image_to_server could be declared in another source file
            // Here you pass the path to the image saved in SPIFFS
            send_image_to_server("/spiffs/capture.jpeg");
        }
    }

	free(receivedMsg);
	free(buf);
	return ret;
}

/* Function to start the web server */
esp_err_t start_webserver(void)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Start the httpd server
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to starting server!");
		return ESP_FAIL;
	}

	// Registering the ws handler
	ESP_LOGI(TAG, "Registering URI handlers");
	httpd_uri_t ws = {
		.uri		= "/ws",
		.method		= HTTP_GET,
		.handler	= get_handler,
		.user_ctx	= NULL,
		.is_websocket = true
	};
	httpd_register_uri_handler(server, &ws);

	return ESP_OK;
}
