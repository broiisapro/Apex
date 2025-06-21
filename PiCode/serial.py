import serial
import time
from datetime import datetime
import os

# === Configuration ===
SERIAL_PORT = '/dev/tty.usbserial-588F0114741'  # Replace with your actual port
BAUD_RATE = 115200              # Replace with your Geiger counter's baud rate
SAVE_FOLDER = 'geiger_logs'
CHUNK_DURATION = 30  # seconds

# Create save folder if it doesn't exist
os.makedirs(SAVE_FOLDER, exist_ok=True)

def get_timestamp():
    return datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

def start_logging():
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
            
            buffer = []
            start_time = time.time()

            while True:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    buffer.append(line)

                # If 30 seconds passed, save to a new file
                if time.time() - start_time >= CHUNK_DURATION:
                    filename = os.path.join(SAVE_FOLDER, f"geiger_{get_timestamp()}.txt")
                    with open(filename, 'w') as f:
                        f.write('\n'.join(buffer))
                    print(f"Saved {len(buffer)} lines to {filename}")
                    
                    buffer = []
                    start_time = time.time()

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("Logging stopped by user.")

if __name__ == '__main__':
    start_logging()
