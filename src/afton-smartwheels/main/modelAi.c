#include "modelAi.h"
#include "esp_dl.h"
#include "esp_log.h"

static const char *TAG = "modelAi";

void setup_model(const char *model_path) {
    // Load the ONNX model using esp-dl
    if (esp_dl_load_model(model_path) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load model");
    } else {
        ESP_LOGI(TAG, "Model loaded successfully");
    }
}

float perform_inference(const char *image_path) {
    // Perform inference using the loaded model and the provided image
    float steering_angle = 0.0f;
    if (esp_dl_infer(image_path, &steering_angle) != ESP_OK) {
        ESP_LOGE(TAG, "Inference failed");
    } else {
        ESP_LOGI(TAG, "Inference successful");
    }
    return steering_angle;
}
