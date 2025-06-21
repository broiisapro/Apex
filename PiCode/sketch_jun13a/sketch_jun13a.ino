#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_MLX90614.h>
#include <QMC5883LCompass.h>

#define MPU_ADDR 0x68

// Sensor instances
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
QMC5883LCompass compass;

// MPU6050 variables
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY;
float gyroAngleX, gyroAngleY, yaw;
float roll, pitch;

float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
unsigned long prevTime;
float elapsedTime;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(1000);

  // MPU6050 init
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0);    // Wake up MPU6050
  Wire.endTransmission(true);

  calculate_IMU_error();

  // Compass init
  compass.init();
  compass.setCalibration(-1537, 1233, -1472, 1384, -1444, 1075);

  // TSL2591 init
  if (!tsl.begin()) {
    Serial.println("TSL2591 not found");
  }

  // MLX90614 init
  if (!mlx.begin()) {
    Serial.println("MLX90614 not found");
  }

  prevTime = millis();
}

void loop() {
  // Time update
  unsigned long currentTime = millis();
  elapsedTime = (currentTime - prevTime) / 1000.0;
  prevTime = currentTime;

  // MPU6050 Read
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  // Adjust with calibration
  GyroX -= GyroErrorX;
  GyroY -= GyroErrorY;
  GyroZ -= GyroErrorZ;

  accAngleX = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 180 / PI - AccErrorX;
  accAngleY = atan(-1 * AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 180 / PI - AccErrorY;

  gyroAngleX += GyroX * elapsedTime;
  gyroAngleY += GyroY * elapsedTime;
  yaw += GyroZ * elapsedTime;

  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  // Compass read
  compass.read();
  int heading = compass.getAzimuth();

  // TSL2591 Lux Sensor
  sensors_event_t event;
  tsl.getEvent(&event);
  float lux = event.light;

  // MLX90614 temp
  float objectTemp = mlx.readObjectTempC();
  float ambientTemp = mlx.readAmbientTempC();

  // Output
  Serial.print("Roll: "); Serial.print(roll, 2);
  Serial.print(" | Pitch: "); Serial.print(pitch, 2);
  Serial.print(" | Yaw: "); Serial.print(yaw, 2);
  Serial.print(" | Heading: "); Serial.print(heading);
  Serial.print(" | Lux: "); Serial.print(lux);
  Serial.print(" | ObjectTemp: "); Serial.print(objectTemp);
  Serial.print(" | AmbientTemp: "); Serial.println(ambientTemp);

  delay(200);
}

void calculate_IMU_error() {
  float AccX_sum = 0, AccY_sum = 0, AccZ_sum = 0;
  float GyroX_sum = 0, GyroY_sum = 0, GyroZ_sum = 0;

  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true);
    float ax = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float ay = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float az = (Wire.read() << 8 | Wire.read()) / 16384.0;

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true);
    float gx = (Wire.read() << 8 | Wire.read()) / 131.0;
    float gy = (Wire.read() << 8 | Wire.read()) / 131.0;
    float gz = (Wire.read() << 8 | Wire.read()) / 131.0;

    AccX_sum += ax;
    AccY_sum += ay;
    AccZ_sum += az;
    GyroX_sum += gx;
    GyroY_sum += gy;
    GyroZ_sum += gz;

    delay(10);
  }

  AccErrorX = atan(AccY_sum / 200.0 / sqrt(pow(AccX_sum / 200.0, 2) + pow(AccZ_sum / 200.0, 2))) * 180 / PI;
  AccErrorY = atan(-AccX_sum / 200.0 / sqrt(pow(AccY_sum / 200.0, 2) + pow(AccZ_sum / 200.0, 2))) * 180 / PI;

  GyroErrorX = GyroX_sum / 200.0;
  GyroErrorY = GyroY_sum / 200.0;
  GyroErrorZ = GyroZ_sum / 200.0;
}
