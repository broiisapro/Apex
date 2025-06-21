import tensorflow as tf
import numpy as np

# Set image size to match training
IMG_SIZE = (224, 224)

# Load your model
model = tf.keras.models.load_model("spectro_model.h5")

# Define class names in order
class_names = ["absorption-carbon", "absorption-helium", "absorption-hydrogen", "absorption-nitrogen", "absorption-oxygen", "emission-carbon", "emission-helium", "emission-hydrogen", "emission-nitrogen", "emission-oxygen"]

# Load and preprocess the image
img_path = 'test_imgs/test_ehelium.jpg'
image = tf.keras.preprocessing.image.load_img(img_path, target_size=IMG_SIZE)
img_array = tf.keras.preprocessing.image.img_to_array(image) / 255.0
img_array = np.expand_dims(img_array, axis=0)

# Make prediction
pred = model.predict(img_array)[0]
thresholded = [class_names[i] for i, val in enumerate(pred) if val > 0.3]
print("Detected elements:", thresholded)
