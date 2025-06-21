import tensorflow as tf
import pandas as pd
import numpy as np
import os
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Dropout
from sklearn.model_selection import train_test_split

# config stuff
IMAGE_DIR = 'images'
CSV_PATH = 'labels.csv'
IMG_SIZE = (224, 224)
BATCH_SIZE = 1
EPOCHS = 10

# csv file
df = pd.read_csv(CSV_PATH)
filenames = df['filename'].values
labels = df.drop(columns=['filename']).values
class_names = df.columns[1:]

# splitting data to val and training data
x_train, x_val, y_train, y_val = train_test_split(filenames, labels, test_size=0.2, random_state=42)

# loading data
def load_image(filename, label):
    image_path = tf.strings.join([IMAGE_DIR, filename], separator=os.sep)
    image_string = tf.io.read_file(image_path)
    image = tf.image.decode_jpeg(image_string, channels=3)
    image = tf.image.resize(image, IMG_SIZE)
    image = image / 255.0  # Normalize to [0, 1]
    return image, label

# creating dataset
train_ds = tf.data.Dataset.from_tensor_slices((x_train, y_train))
train_ds = train_ds.map(load_image).batch(BATCH_SIZE).shuffle(100).prefetch(tf.data.AUTOTUNE)

val_ds = tf.data.Dataset.from_tensor_slices((x_val, y_val))
val_ds = val_ds.map(load_image).batch(BATCH_SIZE).prefetch(tf.data.AUTOTUNE)

# config for model
model = Sequential([
    Conv2D(32, (3, 3), activation='relu', input_shape=(*IMG_SIZE, 3)),
    MaxPooling2D(2, 2),
    Conv2D(64, (3, 3), activation='relu'),
    MaxPooling2D(2, 2),
    Flatten(),
    Dense(128, activation='relu'),
    Dropout(0.5),
    Dense(len(class_names), activation='sigmoid')  # Multi-label output
])

model.compile(optimizer='adam',
              loss='binary_crossentropy',
              metrics=['accuracy'])

# training
model.fit(train_ds, epochs=EPOCHS, validation_data=val_ds)

# save model
model.save('spectro_model.h5')
print("Model saved as 'spectro_model.h5'")
