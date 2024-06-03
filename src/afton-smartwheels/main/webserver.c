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
#include <cJSON.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"

#include "esp_camera.h"
#include "servo_drv.h"

#include "esp_http_server.h"
#include "camera_capture.h"
#include "image_sender.h" // Adjust this include to match your actual file names


static const char *TAG = "WS_SERVER";

// Define your command strings
#define COMMAND_MOV_FWD "MOV_FWD"
#define COMMAND_MOV_STOP "MOV_STOP"
#define COMMAND_MOV_BACK "MOV_BACK"
#define COMMAND_TAKE_IMG "TAKE_IMG"
#define COMMAND_SEND_IMG "SEND_IMG"
#define COMMAND_SW "SW"


#define MOTOR_ENABLE_PIN GPIO_NUM_42
#define MOTOR_IN3_PIN GPIO_NUM_45
#define MOTOR_IN4_PIN GPIO_NUM_46

// Function prototypes
void move_forward(int steering, int speed);
void move_stop(void);
void move_backward(int steering, int speed);
esp_err_t capture_image(httpd_req_t *req, char *imageFileName, size_t imageFileNameSize);
esp_err_t send_image_to_api(httpd_req_t *req);




// esp_err_t get_handler(httpd_req_t *req)
// {
//     char content[100];
//     int ret = httpd_req_recv(req, content, sizeof(content));
//     if (ret <= 0) {
//         if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//             httpd_resp_send_408(req);
//         }
//         return ESP_FAIL;
//     }
    
//     // Parse the JSON message
// 	cJSON *json = cJSON_Parse(content);
//     if (json == NULL) {
//         ESP_LOGE(TAG, "Failed to parse JSON");
//         httpd_resp_send_500(req); // Send Bad Request response
//         return ESP_FAIL;
//     }

//     // Extract the command
//     cJSON *command_item = cJSON_GetObjectItem(json, "command");
//     if (command_item == NULL) {
//         ESP_LOGE(TAG, "No command found in JSON");
//         cJSON_Delete(json);
//         httpd_resp_send_500(req); // Send Bad Request response
//         return ESP_FAIL;
//     }
//     const char *command = command_item->valuestring;

//     // Extract optional parameters
//     cJSON *steering_item = cJSON_GetObjectItem(json, "steering");
//     cJSON *speed_item = cJSON_GetObjectItem(json, "speed");
//     int steering = (steering_item != NULL) ? steering_item->valueint : 0;
//     int speed = (speed_item != NULL) ? speed_item->valueint : 0;

//     // Handle the command
//     if (strcmp(command, COMMAND_MOV_FWD) == 0) {
//         move_forward(steering, speed);
//     } else if (strcmp(command, COMMAND_MOV_STOP) == 0) {
//         move_stop();
//     } else if (strcmp(command, COMMAND_MOV_BACK) == 0) {
//         move_backward(steering, speed);
//     }  else if (strcmp(command, COMMAND_TAKE_IMG) == 0) {
// 		char imageFileName[256];
// 		strcpy(imageFileName, "/spiffs/capture.jpeg");
// 		ret = capture_image(req, imageFileName, sizeof(imageFileName)); // Pass req and imageFileName as parameters
// 		if (ret != ESP_OK) {
// 			ESP_LOGE(TAG, "Failed to capture image");
// 			httpd_resp_send_500(req); // Send Internal Server Error response
// 			cJSON_Delete(json);
// 			return ESP_FAIL;
// 		}
// 	} else if (strcmp(command, COMMAND_SEND_IMG) == 0) {
// 		ret = send_image_to_api(req); // Pass req as parameter
// 		if (ret != ESP_OK) {
// 			ESP_LOGE(TAG, "Failed to send image to API");
// 			httpd_resp_send_500(req); // Send Internal Server Error response
// 			cJSON_Delete(json);
// 			return ESP_FAIL;
// 		}
//     } else {
//         ESP_LOGE(TAG, "Unknown command: %s", command);
//         cJSON_Delete(json);
//         httpd_resp_send_500(req); // Send Bad Request response
//         return ESP_FAIL;
//     }

//     // Clean up and send response
//     cJSON_Delete(json);
//     httpd_resp_sendstr(req, "Command executed");
//     return ESP_OK;
// }

// Example implementations of command functions
void move_forward(int steering, int speed) {
	gpio_set_level(MOTOR_ENABLE_PIN, 1); // Enable the motor
    gpio_set_level(MOTOR_IN3_PIN, 1);
    gpio_set_level(MOTOR_IN4_PIN, 0);
    // Implement your move forward logic here
    ESP_LOGI(TAG, "Moving forward with steering %d and speed %d", steering, speed);
}

void move_stop(void) {
	gpio_set_level(MOTOR_ENABLE_PIN, 0); // Disable the motor
    // Implement your stop logic here
    ESP_LOGI(TAG, "Stopping");
}


void move_backward(int steering, int speed) {
	gpio_set_level(MOTOR_ENABLE_PIN, 1); // Enable the motor
    gpio_set_level(MOTOR_IN3_PIN, 0);
    gpio_set_level(MOTOR_IN4_PIN, 1);
    // Implement your move backward logic here
    ESP_LOGI(TAG, "Moving backward with steering %d and speed %d", steering, speed);
}

void move_steering(int steering) {
    set_servo_angle(steering);
	
     ESP_LOGI(TAG, "Moving steering %d", steering);
}

esp_err_t capture_image(httpd_req_t *req, char *imageFileName, size_t imageFileNameSize) {
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

    // Send by WebSocket
    httpd_ws_frame_t ws_pkt;
    ws_pkt.payload = base64_buffer;
    ws_pkt.len = base64Size;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(base64_buffer);

    return ESP_OK;
}


esp_err_t send_image_to_api(httpd_req_t *req) {
	char imageFileName[256];
	strcpy(imageFileName, "/spiffs/capture.jpeg");
    esp_err_t ret = capture_image(req, imageFileName, sizeof(imageFileName));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to capture image");
        return ret;
    }

    // Send the image to the API
    ESP_LOGI(TAG, "Send the image to the API");
    send_image_to_server("/spiffs/capture.jpeg");
	return ret;
}


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
    }

    // Immediately after receiving the WebSocket message:
    char* receivedMsg = strndup((char*)buf, ws_pkt.len);
    ESP_LOGI(TAG, "Packet final: %d", ws_pkt.final);
    ESP_LOGI(TAG, "Packet fragmented: %d", ws_pkt.fragmented);
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    ESP_LOGI(TAG, "Got packet with message2: [%.*s]", ws_pkt.len, receivedMsg);

    // Parse the JSON message
    cJSON *json = cJSON_Parse(receivedMsg);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON"); // Send Bad Request response
        free(receivedMsg);
        free(buf);
        return ESP_FAIL;
    }

    // Extract the command
    cJSON *command_item = cJSON_GetObjectItem(json, "command");
    if (command_item == NULL) {
        ESP_LOGE(TAG, "No command found in JSON");
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No command found"); // Send Bad Request response
        free(receivedMsg);
        free(buf);
        return ESP_FAIL;
    }
    const char *command = command_item->valuestring;

    // Extract optional parameters
    cJSON *steering_item = cJSON_GetObjectItem(json, "steering");
    cJSON *speed_item = cJSON_GetObjectItem(json, "speed");
    int steering = (steering_item != NULL) ? steering_item->valueint : 0;
    int speed = (speed_item != NULL) ? speed_item->valueint : 0;

    // Handle the command
    if (strcmp(command, COMMAND_MOV_FWD) == 0) {
        move_forward(steering, speed);
    } else if (strcmp(command, COMMAND_MOV_STOP) == 0) {
        move_stop();
    } else if (strcmp(command, COMMAND_MOV_BACK) == 0) {
        move_backward(steering, speed);
    } else if (strcmp(command, COMMAND_SW) == 0) {
        move_steering(steering);
    } else if (strcmp(command, COMMAND_TAKE_IMG) == 0) {
        char imageFileName[256];
        ret = capture_image(req, imageFileName, sizeof(imageFileName)); // Pass req and imageFileName as parameters
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to capture image");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to capture image"); // Send Internal Server Error response
            cJSON_Delete(json);
            free(receivedMsg);
            free(buf);
            return ESP_FAIL;
        }
    } else if (strcmp(command, COMMAND_SEND_IMG) == 0) {
        ret = send_image_to_api(req); // Pass req as parameter
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send image to API");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send image to API"); // Send Internal Server Error response
            cJSON_Delete(json);
            free(receivedMsg);
            free(buf);
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Unknown command: %s", command);
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unknown command"); // Send Bad Request response
        free(receivedMsg);
        free(buf);
        return ESP_FAIL;
    }

    // Clean up and send response
    cJSON_Delete(json);
    free(receivedMsg);
    free(buf);
	httpd_resp_sendstr(req, "Command executed");

    // Example response frame
    // httpd_ws_frame_t ws_pkt_resp;
    // memset(&ws_pkt_resp, 0, sizeof(httpd_ws_frame_t));
    // ws_pkt_resp.payload = (uint8_t *)"Command executed";
    // ws_pkt_resp.len = strlen("Command executed");
    // ws_pkt_resp.type = HTTPD_WS_TYPE_TEXT;
    // return httpd_ws_send_frame(req, &ws_pkt_resp);

    return ESP_OK;
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
