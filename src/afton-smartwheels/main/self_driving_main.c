/* Self-Driving Car Main Program

   Description:
   Main program file for the self-driving car project, developed as part of the World Robot Olympiad (WRO)
   challenge in the Future Engineers category. This program is designed to control the autonomous functions 
   of the car, handling sensor data processing, decision-making, and motor control.

   Filename:
   self_driving_main.c

   License:
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this software is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

   Note:
   This code is part of a project for educational purposes, showcasing basic concepts of autonomous vehicle 
   control, sensor integration, and real-time decision-making. 
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#include "esp_camera.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define WIFI_SSID "SSID1-9980"
#define WIFI_PASS "admin9980"
#define SERVER_URI "http://192.168.1.4:5000/upload"



static const char *TAG = "afton-smartwheels";


// Camera configuration
// Adjust the configuration to match your ESP32-CAM module
camera_config_t camera_config = {
    .pin_pwdn = 8,          // Power down pin
    .pin_reset = 6,         // Reset pin
    .pin_xclk = 13,         // XCLK pin
    .pin_sscb_sda = 21,     // SDA pin
    .pin_sscb_scl = 20,     // SCL pin

    .pin_d7 = 11,           // D7 (Y9)
    .pin_d6 = 12,           // D6 (Y8)
    .pin_d5 = 9,            // D5 (Y7)
    .pin_d4 = 10,           // D4 (Y6)
    .pin_d3 = 18,           // D3 (Y5)
    .pin_d2 = 17,           // D2 (Y4)
    .pin_d1 = 19,           // D1 (Y3)
    .pin_d0 = 16,           // D0 (Y2)
    
    .pin_vsync = 7,         // VSYNC pin
    .pin_href = 9,          // HREF pin
    .pin_pclk = 8,          // PCLK pin

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, 
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,
    .fb_count = 1
};


// Initialize camera
static esp_err_t init_camera() {
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}


// WiFi event handler
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Got IP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retry to connect to the AP");
    }
}


// Connect to Wi-Fi
void wifi_init_sta() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}


/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;


static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    wifi_init_sta();

    // Initialize camera
    ESP_ERROR_CHECK(init_camera());


    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;


        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
        } else {
            // Process the frame buffer (e.g., send over WiFi)
            esp_http_client_config_t config = {
                .url = SERVER_URI,
            };
            esp_http_client_handle_t client = esp_http_client_init(&config);
            esp_http_client_set_method(client, HTTP_METHOD_POST);
            esp_http_client_set_header(client, "Content-Type", "image/jpeg");
            esp_http_client_set_post_field(client, (const char *)fb->buf, fb->len);
            esp_err_t err = esp_http_client_perform(client);

            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Image sent successfully");
            } else {
                ESP_LOGE(TAG, "Failed to send image");
            }

            esp_http_client_cleanup(client);
            esp_camera_fb_return(fb);
        }


        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
