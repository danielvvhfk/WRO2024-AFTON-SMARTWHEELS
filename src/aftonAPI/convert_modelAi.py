# convert_modelAi.py

import tensorflow as tf

# Load the trained Keras model
model = tf.keras.models.load_model('steering_model.h5')

# Convert the model to TensorFlow Lite format
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save the converted model to a .tflite file
with open('steering_model.tflite', 'wb') as f:
    f.write(tflite_model)

print("Model converted to TensorFlow Lite format and saved as 'steering_model.tflite'.")
