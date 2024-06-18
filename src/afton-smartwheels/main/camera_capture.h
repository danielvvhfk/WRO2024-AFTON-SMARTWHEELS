#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <esp_err.h>
#include <stdint.h>

esp_err_t get_image(char *imageFileName, size_t imageFileNameSize);
esp_err_t camera_capture(const char *imageFileName, size_t *pictureSize);

// Declare the functions calcBase64EncodedSize and Image2Base64
int32_t calcBase64EncodedSize(int size);
esp_err_t Image2Base64(const char *imageFileName, size_t base64BufferLen, uint8_t *base64Buffer);

#endif // CAMERA_CAPTURE_H
