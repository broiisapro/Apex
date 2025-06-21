import subprocess
import time
from datetime import datetime
import os

# === Configuration ===
SAVE_FOLDER = 'video_logs'
CHUNK_DURATION = 10  # seconds

# Create save folder if it doesn't exist
os.makedirs(SAVE_FOLDER, exist_ok=True)

def get_timestamp():
    return datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

def record_video(duration=CHUNK_DURATION):
    timestamp = get_timestamp()
    h264_filename = os.path.join(SAVE_FOLDER, f"video_{timestamp}.h264")

    command = [
        "libcamera-vid",
        "-t", str(duration * 1000),
        "-o", h264_filename,
        "--width", "1920",
        "--height", "1080",
        "--framerate", "24",
        "--bitrate", "8000000",
        "--nopreview",
        "--intra", "2"
    ]

    print(f"\n🎥 Recording {h264_filename} for {duration} seconds...")
    subprocess.run(command)
    print(f"Saved {h264_filename}")

def main():
    try:
        while True:
            record_video()
    except KeyboardInterrupt:
        print("\nExiting. Recording session stopped by user.")

if __name__ == "__main__":
    main()

