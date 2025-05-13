# APEX - High-Altitude Spectroscopy Platform

Our project is a high-altitude atmospheric spectroscopy platform that analyzes the composition of the atmosphere at various altitudes using both spectroscopy & machine learning. It brings together hardware sensors, computer vision, and artificial intelligence to develop an system built for atmospheric analysis.

## Project Overview

The system is designed to:
1. Ascend to high altitudes (up to 100,000 feet)
2. Capture spectroscopic images of the atmosphere
3. Analyze the atmospheric composition in post processing
4. Log sensor data and maintain detailed specifications of what, how much, where, everything
5. Process and identify elements using machine learning

## System Components

### Hardware
- Raspberry Pi Zero 2 W (Main Controller)
- Raspberry Pi Camera Module
- IMU (MPU6050) for orientation and acceleration data
- Spectroscopy camera lens
- Triple lithium batteries with voltage boost from 4.5v to 5v
- MicroSD card (32gb?) for data storage
- Styrofoam enclosure with transparent window, really really tiny transparent square just as big as the lens.
- Voltage monitoring system through ground

### Software Components

#### 1. Data Collection (`PiCode/`)
- `main.py`: Core data collection script that:
  - Captures images at specified altitude intervals
  - Logs IMU data
  - Monitors system voltage and temperature
  - Manages data storage
  - Estimates altitude using IMU data

#### 2. Machine Learning Model (`model/`)
- `model.py`: Trains the CNN model for element detection
- `model_run.py`: Production inference script
- `run.py`: Simplified inference script
- Supports detection of:
  - Carbon
  - Helium
  - Hydrogen
  - Nitrogen (sketchy)
  - Oxygen
  - In progress of adding other trace gasses

### Research Documentation

The `research/` directory contains extensive documentation on:
- Spectroscopy principles and implementation
- Sensor and data logging architecture
- Hardware specifications and requirements
- Project background research

## Technical Specifications

### Model Architecture
- Convolutional Neural Network (CNN)
- Input size: 224x224 pixels
- Multi-label classification
- Training parameters:
  - Batch size: 32 (because of high cpu count)
  - Epoch(I dont remember how to spell this): 5-6

### Data Collection Parameters
- Image capture interval: Every 5 feet of altitude
- Maximum altitude: 100,000 feet
- Data logging includes:
  - Timestamp
  - Image name
  - Altitude
  - System voltage
  - Temperature

## Setup and Installation

### Hardware Setup
1. Assemble the hardware components according to the power diagram
2. Install the diffraction grating film on the camera
3. Ensure proper voltage regulation setup
4. Prepare the enclosure with proper insulation

### Software Installation

1. Install Python dependencies:
```bash
cd model
pip install -r requirements.txt
```

2. For Raspberry Pi setup:
```bash
cd PiCode
pip install picamera2 spidev mpu6050-raspberrypi
```

## Usage

1. Train the model (if needed):
```bash
python model/model.py
```

2. Test hardware compatibility:
```bash
python model/hardware_test.py
```

3. Run the main data collection script on the Raspberry Pi:
```bash
python PiCode/main.py
```

## Data Analysis

The system generates:
- Spectroscopic images at regular altitude intervals
- CSV logs with sensor data

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

The project is currently under active development. Major areas for contribution include:
- Improving the machine learning model accuracy
- Enhancing the altitude estimation algorithm
- Optimizing power consumption
- Improving the data collection pipeline