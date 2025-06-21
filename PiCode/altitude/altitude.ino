#include <Wire.h>
#define MPU 0x68

// Accelerometer and Gyroscope variables
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, accZ_g;
float gyroAngleX, gyroAngleY, yaw;
float roll, pitch;

// Error offsets
float AccErrorX, AccErrorY, AccZError;
float GyroErrorX, GyroErrorY, GyroErrorZ;

// Time tracking
unsigned long prevTime;
float elapsedTime;

// Altitude estimation
float velocityZ = 0;
float altitude = 0;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(9600);
  delay(1000);

  calculate_IMU_error();
  prevTime = millis();
}

void loop() {
  // Read raw accelerometer data
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  // Read raw gyroscope data
  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  // Subtract error offsets
  GyroX -= GyroErrorX;
  GyroY -= GyroErrorY;
  GyroZ -= GyroErrorZ;

  // Time difference
  unsigned long currentTime = millis();
  elapsedTime = (currentTime - prevTime) / 1000.0;
  prevTime = currentTime;

  // Angle from accelerometer
  accAngleX = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 180 / PI - AccErrorX;
  accAngleY = atan(-1 * AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 180 / PI - AccErrorY;

  // Integrate gyroscope data
  gyroAngleX += GyroX * elapsedTime;
  gyroAngleY += GyroY * elapsedTime;
  yaw        += GyroZ * elapsedTime;

  // Complementary filter
  roll  = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  // Adjust Z acceleration to remove offset and gravity
  accZ_g = AccZ - AccZError;         // Already in g
  float accZ_m_s2 = (accZ_g - 1.0) * 9.81;  // Subtract gravity and convert to m/s²

  // Integrate vertical velocity and altitude
  velocityZ += accZ_m_s2 * elapsedTime;
  altitude  += velocityZ * elapsedTime;

  // Output values
  Serial.print("Roll: "); Serial.print(roll, 2);
  Serial.print(" | Pitch: "); Serial.print(pitch, 2);
  Serial.print(" | Yaw: "); Serial.print(yaw, 2);
  Serial.print(" | AccZ (g): "); Serial.print(accZ_g + 1.0, 3); // show raw value
  Serial.print(" | Altitude (m): "); Serial.println(altitude, 3);

  delay(20);
}

void calculate_IMU_error() {
  float AccX_sum = 0, AccY_sum = 0, AccZ_sum = 0;
  float GyroX_sum = 0, GyroY_sum = 0, GyroZ_sum = 0;

  for (int i = 0; i < 200; i++) {
    // Read accelerometer
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    float ax = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float ay = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float az = (Wire.read() << 8 | Wire.read()) / 16384.0;

    AccX_sum += ax;
    AccY_sum += ay;
    AccZ_sum += az;

    // Read gyroscope
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    float gx = (Wire.read() << 8 | Wire.read()) / 131.0;
    float gy = (Wire.read() << 8 | Wire.read()) / 131.0;
    float gz = (Wire.read() << 8 | Wire.read()) / 131.0;

    GyroX_sum += gx;
    GyroY_sum += gy;
    GyroZ_sum += gz;

    delay(10);
  }

  float avgAx = AccX_sum / 200.0;
  float avgAy = AccY_sum / 200.0;
  float avgAz = AccZ_sum / 200.0;

  AccErrorX = atan(avgAy / sqrt(avgAx * avgAx + avgAz * avgAz)) * 180 / PI;
  AccErrorY = atan(-avgAx / sqrt(avgAy * avgAy + avgAz * avgAz)) * 180 / PI;
  AccZError = avgAz;

  GyroErrorX = GyroX_sum / 200.0;
  GyroErrorY = GyroY_sum / 200.0;
  GyroErrorZ = GyroZ_sum / 200.0;
}
