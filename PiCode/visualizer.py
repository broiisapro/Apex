from vpython import box, vector, rate, scene
import serial
import math

# 10th change
ser = serial.Serial('/dev/cu.usbmodem11101', 9600, timeout=1)  # why tf does this change every god damn time I plug the arduino into my computer?!?!?!

scene.title = "3D IMU Orientation"
scene.range = 2
scene.forward = vector(-1, -1, -1)
obj = box(length=1, height=0.2, width=0.5, color=vector(0.2, 0.6, 0.9))

def deg_to_rad(deg):
    return deg * math.pi / 180

def get_sensor_data():
    try:
        line = ser.readline().decode('utf-8').strip()
        if "Roll:" in line:
            parts = line.split('|')
            roll = float(parts[0].split(':')[1].strip())
            pitch = float(parts[1].split(':')[1].strip())
            yaw = float(parts[2].split(':')[1].strip())
            return roll, pitch, yaw
    except:
        pass
    return None

while True:
    rate(20)
    result = get_sensor_data()
    if result:
        roll_deg, pitch_deg, yaw_deg = result
        roll = deg_to_rad(roll_deg)
        pitch = deg_to_rad(pitch_deg)
        yaw = deg_to_rad(yaw_deg)

        # Compute rotation vectors
        x_axis = vector(
            math.cos(pitch) * math.cos(yaw),
            math.sin(roll) * math.sin(pitch) * math.cos(yaw) + math.cos(roll) * math.sin(yaw),
            -math.cos(roll) * math.sin(pitch) * math.cos(yaw) + math.sin(roll) * math.sin(yaw)
        )

        up_vector = vector(
            -math.cos(pitch) * math.sin(yaw),
            -math.sin(roll) * math.sin(pitch) * math.sin(yaw) + math.cos(roll) * math.cos(yaw),
            math.cos(roll) * math.sin(pitch) * math.sin(yaw) + math.sin(roll) * math.cos(yaw)
        )

        obj.axis = x_axis
        obj.up = up_vector
