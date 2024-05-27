#ifndef SERVO_DRV_H
#define SERVO_DRV_H

#include "esp_err.h"

#define SERVO_GPIO 39  // GPIO number for the servo control

// Function prototypes
esp_err_t servo_driver_init(void);
esp_err_t set_servo_angle(float angle);

#endif // SERVO_DRV_H
