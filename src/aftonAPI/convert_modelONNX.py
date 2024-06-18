# Convert TensorFlow Model to ONNX

import tf2onnx
import tensorflow as tf

# Load your TensorFlow model
model = tf.keras.models.load_model("steering_model.h5")

# Convert the model to ONNX
onnx_model, _ = tf2onnx.convert.from_keras(model, opset=13)

# Save the ONNX model
with open("steering_model.onnx", "wb") as f:
    f.write(onnx_model.SerializeToString())
