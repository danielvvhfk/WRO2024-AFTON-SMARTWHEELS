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
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "mdns.h"
#include "lwip/inet.h"
#include "lwip/dns.h"


#include "esp_camera.h"
#include "camera_pin.h"
#include "camera_capture.h"
// #include "modelAi.h"
#include "motor_drv.h""


// WiFi credentials
#ifndef WIFI_SSID
#define WIFI_SSID "AFTON"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "123456789"
#endif


#include "image_sender.h"
#include "laser_sensor.h"


#include "servo_drv.h"  // Include the servo driver header

#define SERVO_TEST_TASK_STACK_SIZE 2048
#define SERVO_TEST_TASK_PRIORITY 5


static const char *TAG = "afton-smartwheels";


/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;

#define MOTOR_ENABLE_PIN GPIO_NUM_42
#define MOTOR_IN3_PIN GPIO_NUM_45
#define MOTOR_IN4_PIN GPIO_NUM_46

#define  START_SW_PIN GPIO_NUM_35

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


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

//static camera_config_t camera_config = {
camera_config_t camera_config = {
	.pin_pwdn = CAM_PIN_PWDN,
	.pin_reset = CAM_PIN_RESET,
	.pin_xclk = CAM_PIN_XCLK,
	.pin_sscb_sda = CAM_PIN_SIOD,
	.pin_sscb_scl = CAM_PIN_SIOC,

	.pin_d7 = CAM_PIN_D7,
	.pin_d6 = CAM_PIN_D6,
	.pin_d5 = CAM_PIN_D5,
	.pin_d4 = CAM_PIN_D4,
	.pin_d3 = CAM_PIN_D3,
	.pin_d2 = CAM_PIN_D2,
	.pin_d1 = CAM_PIN_D1,
	.pin_d0 = CAM_PIN_D0,
	.pin_vsync = CAM_PIN_VSYNC,
	.pin_href = CAM_PIN_HREF,
	.pin_pclk = CAM_PIN_PCLK,

	//XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
	.xclk_freq_hz = 20000000,
	.ledc_timer = LEDC_TIMER_0,
	.ledc_channel = LEDC_CHANNEL_0,

	.pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
	.frame_size = FRAMESIZE_VGA, //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

	.jpeg_quality = 12, //0-63 lower number means higher quality
	.fb_count = 1, //if more than one, i2s runs in continuous mode. Use only with JPEG
	.grab_mode = CAMERA_GRAB_WHEN_EMPTY,
	.fb_location = CAMERA_FB_IN_PSRAM
};

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
            wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
            ESP_LOGI(TAG, "Disconnected from SSID:%s, reason:%d", CONFIG_ESP_WIFI_SSID, disconnected->reason);
    
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_init_sta()
{
	s_wifi_event_group = xEventGroupCreate();
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);

	ESP_LOGI(TAG,"ESP-IDF esp_netif");
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *netif = esp_netif_create_default_wifi_sta();
	assert(netif);

#if CONFIG_STATIC_IP

	ESP_LOGI(TAG, "CONFIG_STATIC_IP_ADDRESS=[%s]",CONFIG_STATIC_IP_ADDRESS);
	ESP_LOGI(TAG, "CONFIG_STATIC_GW_ADDRESS=[%s]",CONFIG_STATIC_GW_ADDRESS);
	ESP_LOGI(TAG, "CONFIG_STATIC_NM_ADDRESS=[%s]",CONFIG_STATIC_NM_ADDRESS);

	/* Stop DHCP client */
	ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
	ESP_LOGI(TAG, "Stop DHCP Services");

	/* Set STATIC IP Address */
	esp_netif_ip_info_t ip_info;
	memset(&ip_info, 0 , sizeof(esp_netif_ip_info_t));
	ip_info.ip.addr = ipaddr_addr(CONFIG_STATIC_IP_ADDRESS);
	ip_info.netmask.addr = ipaddr_addr(CONFIG_STATIC_NM_ADDRESS);
	ip_info.gw.addr = ipaddr_addr(CONFIG_STATIC_GW_ADDRESS);;
	esp_netif_set_ip_info(netif, &ip_info);

	/*
	I referred from here.
	https://www.esp32.com/viewtopic.php?t=5380

	if we should not be using DHCP (for example we are using static IP addresses),
	then we need to instruct the ESP32 of the locations of the DNS servers manually.
	Google publicly makes available two name servers with the addresses of 8.8.8.8 and 8.8.4.4.
	*/

	ip_addr_t d;
	d.type = IPADDR_TYPE_V4;
	d.u_addr.ip4.addr = 0x08080808; //8.8.8.8 dns
	dns_setserver(0, &d);
	d.u_addr.ip4.addr = 0x08080404; //8.8.4.4 dns
	dns_setserver(1, &d);

#endif // CONFIG_STATIC_IP

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
	}
	vEventGroupDelete(s_wifi_event_group);
}

void initialize_mdns(void)
{
	//initialize mDNS
	ESP_ERROR_CHECK( mdns_init() );
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set(CONFIG_MDNS_HOSTNAME) );
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);

	//initialize service
	ESP_ERROR_CHECK( mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0) );

#if 0
	//set default mDNS instance name
	ESP_ERROR_CHECK( mdns_instance_name_set("ESP32 with mDNS") );
#endif
}

esp_err_t mountSPIFFS(char * partition_label, char * base_path) {
	ESP_LOGI(TAG, "Initializing SPIFFS file system");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = base_path,
		.partition_label = partition_label,
		.max_files = 5,
		.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return ret;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(partition_label, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	}
	ESP_LOGI(TAG, "Mount SPIFFS filesystem");
	return ret;
}

static void printSPIFFS(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

esp_err_t start_webserver(void);


void servo_test_task(void *arg) {
    // Initialize the servo driver
    esp_err_t ret = servo_driver_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo driver");
        vTaskDelete(NULL);
    }

    // Gradually increase the servo angle
    ESP_LOGI(TAG, "Starting servo test: Increasing angle");
    for (float angle = 0; angle <= SERVO_MAX_DEGREE; angle += 10) {
        ESP_LOGI(TAG, "Setting servo angle to %.2f", angle);
        set_servo_angle(angle);
        vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500ms
    }

    // Gradually decrease the servo angle
    ESP_LOGI(TAG, "Starting servo test: Decreasing angle");
    for (float angle = SERVO_MAX_DEGREE; angle >= 0; angle -= 10) {
        ESP_LOGI(TAG, "Setting servo angle to %.2f", angle);
        set_servo_angle(angle);
        vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500ms
    }

    // Task complete
    ESP_LOGI(TAG, "Servo test completed");
    vTaskDelete(NULL);  // Delete the task after completion
}


void configure_start_switch() {
    gpio_config_t io_conf2;
    
    // Configure GPIO for the start switch
    io_conf2.intr_type = GPIO_INTR_DISABLE;          // No interrupts
    io_conf2.mode = GPIO_MODE_INPUT;                 // Set as input mode
    io_conf2.pin_bit_mask = (1ULL << START_SW_PIN);  // Pin bit mask for GPIO 35
    io_conf2.pull_down_en = GPIO_PULLDOWN_DISABLE;   // Disable pull down
    io_conf2.pull_up_en = GPIO_PULLUP_ENABLE;        // Enable pull up
    
    gpio_config(&io_conf2);                          // Configure the GPIO with the settings
}

void wait_for_start_button() {
    int level = gpio_get_level(START_SW_PIN);
    while (level == 1) {
        // Add a delay to avoid flooding the log
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        level = gpio_get_level(START_SW_PIN);  // Check the button state again
    }
}


void app_main(void)
{

	// gpio_config_t io_conf;

    // // Configure GPIOs for motor control
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // io_conf.pin_bit_mask = (1ULL << MOTOR_ENABLE_PIN) | (1ULL << MOTOR_IN3_PIN) | (1ULL << MOTOR_IN4_PIN);
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // gpio_config(&io_conf);

	// init motor
	motor_driver_init();

	// Configure the start switch GPIO
    configure_start_switch();


    /* Configure the peripheral according to the LED type */
    configure_led();
    blink_led();

   // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Initilize WiFi
	// wifi_init_sta();

	// Initialize mDNS
	// initialize_mdns();

	// Mount SPIFFS
	char *partition_label = "storage";
	char *base_path = "/spiffs";
	ret = mountSPIFFS(partition_label, base_path);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "mountSPIFFS fail");
		while(1) { vTaskDelay(1); }
	}
	printSPIFFS("/spiffs/");

	// Initialize camera
	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Camera Init Failed");
		while(1) { vTaskDelay(1); }
	}

	/* Start the server */
	// start_webserver();
	// Setup the ONNX model
    // const char *model_path = "/spiffs/models/steering_model.onnx";
    // setup_model(model_path);


	 // Initialize the servo driver
    ret = servo_driver_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo driver");
    }

<<<<<<< HEAD
	err = motor_driver_init();
    if (err != ESP_OK) {
        ESP_LOGE("APP_MAIN", "Motor driver initialization failed");
        return;
    }


    // Initialize the laser sensor
    ret = laser_sensor_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize laser sensor");
        return;
    }

    ret = laser_sensor_start_ranging();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start ranging");
        return;
    }

    uint16_t distance;

	int steering=90;
	set_servo_angle(steering);
	ESP_LOGI(TAG, "Moving steering %d", steering);
	   

=======
	
>>>>>>> parent of ef455e3 (fix some issues)
    // Wait for the start button to be pressed
    printf("Waiting for start button to be pressed...\n");
    wait_for_start_button();
    printf("Start button pressed, continuing...\n");

	int press_count = 0;
    while (press_count < 5) {
        char imageFileName[256];
        strcpy(imageFileName, "/spiffs/capture.jpeg");
        esp_err_t ret = get_image(imageFileName, sizeof(imageFileName));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to capture image");
            return;
        }

<<<<<<< HEAD

	int lap_count = 0;
	int sector_count = 2;

	while (lap_count < 4) {
		// Define the steering angle for each sector
		int steering;
		if (sector_count % 2 == 0) {
			// Straight sector
			steering = 95;
			move_forward(steering, 68); 
		}else {
			// Turn sector
			steering = 122;
			move_forward(steering, 70); // Adjust speed as necessary
			vTaskDelay(840 / portTICK_PERIOD_MS);
		}

		// Set the steering angle
		set_servo_angle(steering);
		ESP_LOGI(TAG, "Moving steering %d", steering);


		// Capture image (simulate)
		// char imageFileName[256];
		// strcpy(imageFileName, "/spiffs/capture.jpeg");
		// esp_err_t ret = get_image(imageFileName, sizeof(imageFileName));
		// if (ret != ESP_OK) {
		// 	ESP_LOGE(TAG, "Failed to capture image");
		// 	return;
		// }

		// Increase sector count
		sector_count++;

		// Check if a lap is completed
		if (sector_count >=8 ) {
			lap_count++;
			sector_count = 0; // Reset sector count for next lap
			printf("Lap %d completed.\n", lap_count);

		 
		}

		ret = laser_sensor_get_distance(&distance);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Distance: %d mm", distance);
        } else {
            ESP_LOGE(TAG, "Failed to get distance");
        }

		// Add a delay to simulate the vehicle moving through the sector
		if (sector_count % 2 == 0) {
			// Straight sector
			vTaskDelay(1875  / portTICK_PERIOD_MS);
		
			 // Adjust the delay as necessary
		}
	}

	// int press_count = 0;
    // while (press_count < 5) {
    //     char imageFileName[256];
    //     strcpy(imageFileName, "/spiffs/capture.jpeg");
    //     esp_err_t ret = get_image(imageFileName, sizeof(imageFileName));
    //     if (ret != ESP_OK) {
    //         ESP_LOGE(TAG, "Failed to capture image");
    //         return;
    //     }

	// 	// move forward
	// 	move_forward(90, 70);
	// 	press_count++;
=======
		// move forward
		move_forward(90, 70);
		press_count++;
>>>>>>> parent of ef455e3 (fix some issues)
		 
		


        // // Perform inference
        // float steering_angle = perform_inference(imageFileName);

        // // Print the inferred steering angle
        // printf("Inferred Steering Angle: %f\n", steering_angle);

        // Add a delay to avoid flooding the log
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }	// Create a task to test the servo
    // xTaskCreate(servo_test_task, "servo_test_task", SERVO_TEST_TASK_STACK_SIZE, NULL, SERVO_TEST_TASK_PRIORITY, NULL);
   
	// Call move_stop function after button has been pressed 10 times
    move_stop();
   
    // Assuming you receive a command to capture and send an image
    // send_image_to_server("/spiffs/capture.jpeg");

	

    

	// while (1) {
    //     ret = laser_sensor_get_distance(&distance);
    //     if (ret == ESP_OK) {
    //         ESP_LOGI(TAG, "Distance: %d mm", distance);
    //     } else {
    //         ESP_LOGE(TAG, "Failed to get distance");
    //     }
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay 1 second
    // }

    // ret = laser_sensor_stop_ranging();
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to stop ranging");
    // }

	 // while (1) {
    //     ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
    //     blink_led();
    //     /* Toggle the LED state */
    //     s_led_state = !s_led_state;

    //     vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    // }
}
