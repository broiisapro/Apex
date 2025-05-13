import os
import time
import spidev
from picamera2 import Picamera2
from datetime import datetime
from mpu6050 import mpu6050

# Config stuff, directories are based on what I remember, but will change if need to when I get my hands on the pi
# Where to save the images
IMAGE_DIR = "/home/pi/altitude_captures"
# Where to save the log data
LOG_FILE = "/home/pi/altitude_log.csv"
# Take a picture every 5 feet
ALTITUDE_STEP_FEET = 5
# Stop after 100,000 feet
MAX_ALTITUDE_FEET = 100000
# Voltage reference for ADC
VREF = 5
# Channel to read voltage from
ADC_CHANNEL = 0

# Setup stuff
# Make the image folder if it doesn't exist
os.makedirs(IMAGE_DIR, exist_ok=True)

# Initialize the camera
camera = Picamera2()
# Start the camera
camera.start()

# Setup the ACS712 which is a voltage sensor
spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1350000

# Connect to IMU (MPU6050 on default I2C address)
imu = mpu6050(0x68)

# Create the log file and write the headers
with open(LOG_FILE, "w") as log:
    log.write("timestamp,image_name,altitude_ft,voltage,temp_C\n")

# Reading the voltage
def read_voltage(channel=0):
    adc = spi.xfer2([1, (8 + channel) << 4, 0])
    value = ((adc[1] & 3) << 8) + adc[2]
    raw_voltage = (value * VREF) / 1023.0
    return raw_voltage

# Cpu temps for the pi
def get_cpu_temp():
    with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
        return int(f.read()) / 1000.0

# altitude estimation
def estimate_altitude(accel_z, prev_vz, dt, altitude_ft):
    # Acceleration due to gravity (m/s²)
    g = 9.81
    # Subtract gravity
    az = accel_z - 1.0
    # Update vertical velocity
    vz = prev_vz + az * dt
    # Update altitude in meters
    altitude_m = altitude_ft * 0.3048 + vz * dt
    # Convert meters back to feet
    return vz, altitude_m / 0.3048

# main loop from here

# Initial values
prev_time = time.time()
# Start with 0 vertical velocity and altitude
prev_vz = 0.0
altitude_ft = 0.0
next_capture_alt = ALTITUDE_STEP_FEET
image_count = 0

while altitude_ft <= MAX_ALTITUDE_FEET:
    # Calculate time difference between loop cycles
    curr_time = time.time()
    dt = curr_time - prev_time
    prev_time = curr_time

    # Get accelerometer reading from IMU
    accel_data = imu.get_accel_data()
    # We care only about the vertical axis
    accel_z = accel_data['z']

    # Estimate new altitude using basic physics
    prev_vz, altitude_ft = estimate_altitude(accel_z, prev_vz, dt, altitude_ft)

    # if it has increased by 5 feet, take a picture
    if altitude_ft >= next_capture_alt:
        image_count += 1
        next_capture_alt += ALTITUDE_STEP_FEET

        # Generate a file name and timestamp
        img_name = f"img_{image_count:05d}.jpg"
        img_path = os.path.join(IMAGE_DIR, img_name)
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

        # Take the photo
        camera.capture_file(img_path)

        # Measure voltage and CPU temperature
        voltage = read_voltage()
        temp = get_cpu_temp()

        # Save the log entry
        with open(LOG_FILE, "a") as log:
            log.write(f"{timestamp},{img_name},{altitude_ft:.1f},{voltage:.2f},{temp:.2f}\n")

        # testing stuff, probably wont need this
        #print(f"[{image_count}] {img_name}: Altitude={altitude_ft:.1f} ft | Voltage={voltage:.2f} V | Temp={temp:.1f} °C")
