"""
training_modelAi.py

This script is designed to train a machine learning model to predict steering angles for an autonomous vehicle
based on images captured from the vehicle's camera. The steering angles are indicated in the filenames of the images.
For example, '001_right_170.jpg' indicates a right turn with a steering angle of 170 degrees.

1. The images are stored in a folder named 'data/images'.
2. The filenames have a prefix number followed by the steering direction and angle (e.g., '001_right_170.jpg').

Usage:
    python training_modelAi.py

Dependencies:
    - tensorflow
    - numpy
    - opencv-python
    - matplotlib
    - scikit-learn

Make sure to install the dependencies using:
    pip install tensorflow numpy opencv-python matplotlib scikit-learn
"""

import tensorflow as tf
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from sklearn.model_selection import train_test_split
import os
import numpy as np
import matplotlib.pyplot as plt
# Function to extract steering angle from filename


# def get_label_from_filename(filename):
#     try:
#         # Extract the file name part (after the last backslash)
#         base_name = os.path.basename(filename)
#         parts = base_name.split('_')
        
#         if len(parts) == 2 and 'straight' in parts[1]:
#             # Handle straight direction separately
#             return 90 / 180.0  # Normalize the angle to [0, 1]
        
#         if len(parts) != 3:
#             raise ValueError(f"Unexpected filename format: {filename}")
        
#         direction = parts[1]
#         angle = int(parts[2].split('.')[0])  # Adjusted to use the third part for the angle
        
#         if direction == 'left':
#             return angle / 180.0  # Normalize the angle to [0, 1]
#         elif direction == 'right':
#             return (angle + 90) / 180.0  # Normalize the angle to [0, 1]
#         else:
#             raise ValueError(f"Unexpected direction in filename: {filename}")
#     except Exception as e:
#         print(f"Filename format error for {filename}: {e}")
#         raise


# Function to extract steering angle from filename
def get_label_from_filename(filename):
    try:
        # Extract the file name part (after the last backslash)
        base_name = os.path.basename(filename)
        parts = base_name.split('_')
        
        if len(parts) == 2 and 'straight' in parts[1]:
            # Handle straight direction separately
            return 90 / 180.0  # Normalize the angle to [0, 1]
        
        if len(parts) != 3:
            raise ValueError(f"Unexpected filename format: {filename}")
        
        direction = parts[1]
        angle = int(parts[2].split('.')[0])  # Adjusted to use the third part for the angle
        
        if direction == 'left':
            if angle < 10 or angle > 89:
                raise ValueError(f"Angle {angle} out of range for direction 'left'")
            return angle / 180.0  # Normalize the angle to [0, 1]
        elif direction == 'right':
            if angle < 91 or angle > 155:
                raise ValueError(f"Angle {angle} out of range for direction 'right'")
            return angle / 180.0  # Normalize the angle to [0, 1]
        else:
            raise ValueError(f"Unexpected direction in filename: {filename}")
    except Exception as e:
        print(f"Filename format error for {filename}: {e}")
        raise

# # Check if the directory exists and list files
# data_dir = 'data/images'
# if not os.path.exists(data_dir):
#     print(f"Directory {data_dir} does not exist.")
# else:
#     print(f"Directory {data_dir} exists. Listing files:")
#     for root, dirs, files in os.walk(data_dir):
#         for filename in files:
#             print(filename)


# Initialize ImageDataGenerator with rescaling and validation split
datagen = ImageDataGenerator(rescale=1./255, validation_split=0.2)

# Load and preprocess the training dataset
train_generator = datagen.flow_from_directory(
    'data/images',     # Directory where the images are stored
    target_size=(50, 50),  # Resize all images to 50x50 pixels (width, height) maintaining aspect ratio
    batch_size=32,     # Number of images to process in each batch
    class_mode=None,   # Since labels are handled separately, no class labels are used
    subset='training', # Specify this is the training subset
    shuffle=False      # Do not shuffle images (useful for controlled batch order)
)

# Load and preprocess the validation dataset
validation_generator = datagen.flow_from_directory(
    'data/images',     # Directory where the images are stored
    target_size=(50, 50),  # Resize all images to 50x50 pixels (width, height) maintaining aspect ratio
    batch_size=32,     # Number of images to process in each batch
    class_mode=None,   # Since labels are handled separately, no class labels are used
    subset='validation', # Specify this is the validation subset
    shuffle=False      # Do not shuffle images (useful for controlled batch order)
)

# Debugging: Print filenames to check format
# Debugging: Print filenames to check format
print("Training Filenames from the generator:", train_generator.filenames)
print("Validation Filenames from the generator:", validation_generator.filenames)

# Extract labels from filenames for training and validation sets
train_labels = np.array([get_label_from_filename(f) for f in train_generator.filenames])
validation_labels = np.array([get_label_from_filename(f) for f in validation_generator.filenames])

# Create custom data generators
class CustomDataGenerator(tf.keras.utils.Sequence):
    def __init__(self, image_generator, labels, batch_size):
        self.image_generator = image_generator
        self.labels = labels
        self.batch_size = batch_size

    def __len__(self):
        return len(self.image_generator)

    def __getitem__(self, idx):
        batch_x = self.image_generator[idx]
        batch_y = self.labels[idx * self.batch_size:(idx + 1) * self.batch_size]
        return batch_x, batch_y

train_data_generator = CustomDataGenerator(train_generator, train_labels, batch_size=32)
validation_data_generator = CustomDataGenerator(validation_generator, validation_labels, batch_size=32)

# Define the convolutional neural network model
model = tf.keras.models.Sequential([
    tf.keras.layers.Conv2D(32, (3, 3), activation='relu', input_shape=(50, 50, 3)),  # First convolutional layer
    tf.keras.layers.MaxPooling2D((2, 2)),          # Max pooling layer
    tf.keras.layers.Conv2D(64, (3, 3), activation='relu'),  # Second convolutional layer
    tf.keras.layers.MaxPooling2D((2, 2)),          # Max pooling layer
    tf.keras.layers.Conv2D(128, (3, 3), activation='relu'), # Third convolutional layer
    tf.keras.layers.MaxPooling2D((2, 2)),          # Max pooling layer
    tf.keras.layers.Flatten(),                     # Flatten layer to convert 2D to 1D
    tf.keras.layers.Dense(512, activation='relu'), # Fully connected dense layer
    tf.keras.layers.Dense(1, activation='sigmoid') # Output layer for normalized steering angle
])

# Compile the model with mean squared error loss and mean absolute error metric
model.compile(optimizer='adam', loss='mse', metrics=['mae'])

# Train the model and store the training history
history = model.fit(
    train_data_generator, 
    validation_data=validation_data_generator, 
    epochs=10
)

# Plot the training and validation loss over epochs
plt.plot(history.history['loss'], label='loss')
plt.plot(history.history['val_loss'], label='val_loss')
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.legend()
plt.grid(True)
plt.show()

# Evaluate the model on the validation dataset
test_loss, test_mae = model.evaluate(validation_data_generator)
print(f'Test Loss: {test_loss}')
print(f'Test MAE: {test_mae}')

# Save the trained model to a file
model.save('steering_model.h5')
