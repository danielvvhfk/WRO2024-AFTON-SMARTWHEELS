#ifndef MODELAI_H
#define MODELAI_H

#include "esp_dl/nn.h" // Include necessary headers from esp-dl
#include "esp_dl/tvm.h"

void setup_model(const char *model_path);
float perform_inference(const char *image_path);

#endif // MODELAI_H