#ifndef SERVO_DRV_H
#define SERVO_DRV_H

#include "esp_err.h"

#define SERVO_GPIO 39  // GPIO number for the servo control
#define SERVO_MIN_PULSEWIDTH_US 1000  // Minimum pulse width in microseconds
#define SERVO_MAX_PULSEWIDTH_US 2000  // Maximum pulse width in microseconds
#define SERVO_MAX_DEGREE 180          // Maximum angle in degrees


// Function prototypes
esp_err_t servo_driver_init(void);
esp_err_t set_servo_angle(float angle);

#endif // SERVO_DRV_H
