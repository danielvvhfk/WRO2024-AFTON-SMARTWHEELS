#ifndef MODEL_AI_H
#define MODEL_AI_H

void setup_model(const char *model_path);
float perform_inference(const char *image_path);
void cleanup_model();

#endif // MODEL_AI_H
