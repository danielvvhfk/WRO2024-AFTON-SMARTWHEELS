#include "servo_drv.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "SERVO";

#define SERVO_MIN_PULSEWIDTH_US 1000  // Minimum pulse width in microseconds
#define SERVO_MAX_PULSEWIDTH_US 2000  // Maximum pulse width in microseconds
#define SERVO_MAX_DEGREE 180          // Maximum angle in degrees

esp_err_t servo_driver_init(void) {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_16_BIT,
        .freq_hz          = 50,  // Frequency in Hertz (50Hz for servos)
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_GPIO,
        .duty           = 0, // Set duty to 0 initially
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    return ESP_OK;
}

esp_err_t set_servo_angle(float angle) {
    if (angle < 0 || angle > SERVO_MAX_DEGREE) {
        ESP_LOGE(TAG, "Angle out of range");
        return ESP_ERR_INVALID_ARG;
    }

    // Calculate the pulse width in microseconds
    uint32_t pulsewidth_us = SERVO_MIN_PULSEWIDTH_US +
                             ((SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) * angle) / SERVO_MAX_DEGREE;

    // Calculate the duty cycle value
    uint32_t duty = (pulsewidth_us * (1 << LEDC_TIMER_16_BIT)) / 20000;  // 20ms period (50Hz)

    // Set the duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0));

    return ESP_OK;
}
