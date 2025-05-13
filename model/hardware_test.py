import tensorflow as tf
print("TensorFlow version:", tf.__version__)
print("Num GPUs Available:", len(tf.config.list_physical_devices('GPU')))
print("Num CPUs Available:", len(tf.config.list_physical_devices('CPU')))
print("Num Logical CPUs Available:", len(tf.config.list_logical_devices('CPU')))
print("Num Logical GPUs Available:", len(tf.config.list_logical_devices('GPU')))
print("Num Physical GPUs Available:", len(tf.config.list_physical_devices('GPU')))
print("Num Physical CPUs Available:", len(tf.config.list_physical_devices('CPU')))