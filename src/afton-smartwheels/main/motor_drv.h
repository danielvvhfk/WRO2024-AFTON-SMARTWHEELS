#ifndef MOTOR_DRV_H
#define MOTOR_DRV_H

#include "esp_err.h"

#define MOTOR_ENABLE_PIN GPIO_NUM_42
#define MOTOR_IN3_PIN GPIO_NUM_45
#define MOTOR_IN4_PIN GPIO_NUM_46


#define LEDC_TIMER              LEDC_TIMER_1
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          MOTOR_ENABLE_PIN
#define LEDC_CHANNEL            LEDC_CHANNEL_1
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY          5000 // Frequency in Hertz

esp_err_t motor_driver_init(void);
esp_err_t move_forward(int steering, int speed);
esp_err_t move_stop(void);
esp_err_t move_backward(int steering, int speed);

#endif // MOTOR_DRV_H
