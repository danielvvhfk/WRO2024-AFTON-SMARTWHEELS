#include "motor_drv.h"
#include "esp_log.h"
#include <driver/gpio.h>
#include <driver/ledc.h>

static const char *TAG = "MOTOR_DRV";

esp_err_t motor_driver_init(void) {
    // Configure GPIOs for motor control
    ESP_LOGI(TAG, "Configure GPIOs for motor control");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << MOTOR_ENABLE_PIN) | (1ULL << MOTOR_IN3_PIN) | (1ULL << MOTOR_IN4_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Motor GPIO config failed");
        return err;
    }

    // Initialize LEDC for PWM
    ESP_LOGI(TAG, "Initialize LEDC for PWM");
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LEDC timer config failed");
        return err;
    }

    ESP_LOGI(TAG, "Initialize LEDC channel");
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0 initially
        .hpoint         = 0
    };
    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LEDC channel config failed");
        return err;
    }

    return ESP_OK;
}

esp_err_t move_forward(int steering, int speed) {
    int duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 100; // Convert speed percentage to duty cycle
    esp_err_t err = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Set duty failed");
        return err;
    }
    err = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Update duty failed");
        return err;
    }

    gpio_set_level(MOTOR_IN3_PIN, 1);
    gpio_set_level(MOTOR_IN4_PIN, 0);

    ESP_LOGI(TAG, "Moving forward with steering %d and speed %d", steering, speed);
    return ESP_OK;
}

esp_err_t move_stop(void) {
    esp_err_t err = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Set duty to 0 failed");
        return err;
    }
    err = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Update duty failed");
        return err;
    }

    gpio_set_level(MOTOR_IN3_PIN, 0);
    gpio_set_level(MOTOR_IN4_PIN, 0);

    ESP_LOGI(TAG, "Motor stopped");
    return ESP_OK;
}

esp_err_t move_backward(int steering, int speed) {
    int duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 100; // Convert speed percentage to duty cycle
    esp_err_t err = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Set duty failed");
        return err;
    }
    err = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Update duty failed");
        return err;
    }

    gpio_set_level(MOTOR_IN3_PIN, 0);
    gpio_set_level(MOTOR_IN4_PIN, 1);

    ESP_LOGI(TAG, "Moving backward with steering %d and speed %d", steering, speed);
    return ESP_OK;
}
