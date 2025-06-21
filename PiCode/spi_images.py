import serial

# Update to match your serial port and baud rate
PORT = "/dev/cu.usbserial-0001"  # or /dev/ttyUSB0 on Linux
BAUD = 115200
TIMEOUT = 20
OUTPUT_FILE = "captured.jpg"

def receive_image():
    with serial.Serial(PORT, BAUD, timeout=TIMEOUT) as ser:
        with open(OUTPUT_FILE, "wb") as f:
            print("Waiting for image...")

            # Wait for start of image (FFD8)
            while True:
                byte = ser.read()
                if not byte:
                    print("Timeout or serial closed.")
                    return

                if byte == b'\xff':
                    byte2 = ser.read()
                    if byte2 == b'\xd8':
                        f.write(byte)
                        f.write(byte2)
                        print("Found JPEG start marker.")
                        break

            # Read image until end marker (FFD9)
            while True:
                byte = ser.read()
                if not byte:
                    print("Timeout or serial closed before image complete.")
                    return

                f.write(byte)
                if byte == b'\xff':
                    byte2 = ser.read()
                    if not byte2:
                        print("Timeout during image end check.")
                        return
                    f.write(byte2)
                    if byte2 == b'\xd9':
                        print("Found JPEG end marker. Image saved.")
                        break

if __name__ == "__main__":
    receive_image()
