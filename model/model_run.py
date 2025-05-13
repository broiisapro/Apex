import tensorflow as tf
import numpy as np

# Set image size to match training
IMG_SIZE = (224, 224)

# Load your model
model = tf.keras.models.load_model("spectro_model.h5")
# Check if the model is loaded correctly
print("Model loaded successfully.")
# Check if the model is a valid Keras model
if not isinstance(model, tf.keras.Model):
    raise ValueError("The loaded model is not a valid Keras model.")
# Check the model summary
model.summary()
# Check the model input shape
input_shape = model.input_shape
print("Model input shape:", input_shape)
# Check the model output shape
output_shape = model.output_shape
print("Model output shape:", output_shape)
# Check the model architecture
model_layers = [layer.name for layer in model.layers]
print("Model layers:", model_layers)

# Define class names in order
class_names = ["carbon", "helium", "hydrogen", "nitrogen", "oxygen"]  # <-- adjust for your dataset

# Load and preprocess the image
img_path = 'test_imgs/test_1.jpg'
image = tf.keras.preprocessing.image.load_img(img_path, target_size=IMG_SIZE)
img_array = tf.keras.preprocessing.image.img_to_array(image) / 255.0
img_array = np.expand_dims(img_array, axis=0)

# Make prediction
pred = model.predict(img_array)[0]
thresholded = [class_names[i] for i, val in enumerate(pred) if val > 0.5]
print("Detected elements:", thresholded)
