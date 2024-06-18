"""
test_modelAi.py

This script is designed to test a trained machine learning model for predicting steering angles
based on a single sample image. The model is trained to predict the steering angle for an autonomous vehicle.

1. The script loads a pre-trained model from 'steering_model.h5'.
2. It preprocesses a sample image ('captured.jpg').
3. The script predicts the steering angle for the image and prints the result.

Usage:
    python test_modelAi.py

Dependencies:
    - tensorflow
    - numpy
    - opencv-python
    - matplotlib

Make sure to install the dependencies using:
    pip install tensorflow numpy opencv-python matplotlib
"""

import tensorflow as tf
import numpy as np
import cv2
import matplotlib.pyplot as plt

# Load the trained model
model = tf.keras.models.load_model('steering_model.h5')

# Function to preprocess the image
def preprocess_image(image_path):
    img = cv2.imread(image_path)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    print(f"Original image shape: {img.shape}")  # Print original shape
    img = cv2.resize(img, (50, 50))  # Resize to match training dimensions (width, height)
    img = img / 255.0  # Normalize to [0, 1]
    img = np.expand_dims(img, axis=0)  # Add batch dimension
    print(f"Preprocessed image shape: {img.shape}")  # Verify shape
    return img

# # Function to convert normalized steering angle to degrees
# def normalized_to_degrees(normalized_angle):
#     return normalized_angle * 180.0
# Function to convert normalized steering angle to degrees
def normalized_to_degrees(normalized_angle):
    return int(round(normalized_angle * 180.0))

# Load and preprocess the image
image_path = 'captured.jpg'
preprocessed_image = preprocess_image(image_path)

# Verify the preprocessed image shape
# expected_shape = (1, 50, 50, 3)  # Match the expected shape (batch size, height, width, channels)
# if preprocessed_image.shape != expected_shape:
#     raise ValueError(f"Incorrect image shape: expected {expected_shape}, got {preprocessed_image.shape}")

# Predict the steering angle
predicted_angle = model.predict(preprocessed_image)[0][0]

# Convert the normalized angle to degrees
steering_angle_degrees = normalized_to_degrees(predicted_angle)

# Print the result
print(f'Steering Angle: {steering_angle_degrees:.2f} degrees')

# Display the image and the predicted angle
plt.imshow(cv2.cvtColor(cv2.imread(image_path), cv2.COLOR_BGR2RGB))
plt.title(f'Steering Angle: {steering_angle_degrees:.2f} degrees')
plt.axis('off')
plt.show()
