#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <stdio.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Take a picture and save it to a file.
 *
 * @param FileName The name of the file to save the picture to.
 * @param pictureSize Pointer to store the size of the taken picture.
 * @return esp_err_t ESP_OK on success, ESP_FAIL on error.
 */
esp_err_t camera_capture(char *FileName, size_t *pictureSize);

/**
 * @brief Convert an image file to a base64 encoded string.
 *
 * @param imageFileName Name of the image file to convert.
 * @param base64_buffer_len Length of the buffer allocated for the base64 string.
 * @param base64_buffer Buffer to store the base64 encoded string.
 * @return esp_err_t ESP_OK on success, ESP_FAIL on error.
 */
esp_err_t Image2Base64(char *imageFileName, size_t base64_buffer_len, uint8_t *base64_buffer);

// Declaration of calcBase64EncodedSize
// Assuming calcBase64EncodedSize takes an integer and returns an integer
int calcBase64EncodedSize(int inputSize);


#ifdef __cplusplus
}
#endif

#endif // CAMERA_CAPTURE_H
