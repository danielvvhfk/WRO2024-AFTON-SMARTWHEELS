idf_component_register(SRCS  "motor_drv.c" "servo_drv.c"  "laser_sensor.c" "self_driving_main.c" "webserver.c" "image_sender.c" "camera_capture.c"
                       INCLUDE_DIRS "."
                       REQUIRES st_vl53l4cd  mbedtls esp_http_server cJSON esp_wifi nvs_flash spiffs esp_http_client)

