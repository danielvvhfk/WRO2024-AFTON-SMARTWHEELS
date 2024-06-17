#include <stdio.h>
#include "esp_dl.h"
#include "esp_log.h"
#include "modelAi.h"

static const char *TAG = "MODEL_AI";
static esp_dl_model_handle_t model_handle = NULL;

void setup_model(const char *model_path) {
    // Load the ONNX model
    model_handle = esp_dl_load_model(model_path);
    if (model_handle == NULL) {
        ESP_LOGE(TAG, "Failed to load model");
    } else {
        ESP_LOGI(TAG, "Model loaded successfully");
    }
}

float perform_inference(const char *image_path) {
    if (model_handle == NULL) {
        ESP_LOGE(TAG, "Model is not loaded");
        return -1;
    }

    // Load and preprocess the image
    // Example preprocessing, replace with actual logic
    float input_data[YOUR_INPUT_SIZE];  // Adjust to your model input size
    // Load and preprocess the image into input_data here

    float output_data[1];  // Assuming a single output value for steering angle
    esp_dl_run_inference(model_handle, input_data, output_data);

    return output_data[0];
}

void cleanup_model() {
    if (model_handle != NULL) {
        esp_dl_unload_model(model_handle);
        model_handle = NULL;
        ESP_LOGI(TAG, "Model unloaded successfully");
    }
}
