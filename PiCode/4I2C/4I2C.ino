#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_MLX90614.h>
#include <QMC5883LCompass.h>
#include <Adafruit_VEML6075.h>
#include <Adafruit_BME280.h>

#define MPU 0x68
#define UV_SENSOR_PIN 35
#define MIC_PIN 34

// --- IMU Variables ---
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, accZ_g;
float gyroAngleX, gyroAngleY, yaw;
float roll, pitch;
float AccErrorX, AccErrorY, AccZError;
float GyroErrorX, GyroErrorY, GyroErrorZ;
unsigned long prevTime;
float elapsedTime;
float velocityZ = 0;
float altitude = 0;

// --- Sensor Objects ---
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
QMC5883LCompass compass;
Adafruit_VEML6075 uv = Adafruit_VEML6075();
Adafruit_BME280 bme;

// --- Light Sensor ---
uint32_t full, ir, vis;

// --- Magnetometer ---
int magX, magY, magZ;
String direction;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  analogReadResolution(12);

  // MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); Wire.write(0);  // Wake up sensor
  Wire.endTransmission(true);
  calculate_IMU_error();
  prevTime = millis();

  // TSL2591
  if (!tsl.begin()) Serial.println("TSL2591 not found!");
  else {
    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
  }

  // MLX90614
  if (!mlx.begin()) Serial.println("MLX90614 not found!");

  // QMC5883L
  compass.init();
  compass.setCalibration(-2786, 2237, -3003, 2435, -1038, 1412);

  // VEML6075
  if (!uv.begin()) {
    Serial.println("VEML6075 not found!");
    while (1);
  }

  // BME280
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found!");
    while (1);
  }
}

void loop() {
  readMPU6050();
  compass.read();

  // QMC5883L
  magX = compass.getX();
  magY = compass.getY();
  magZ = compass.getZ();
  float heading = compass.getAzimuth();
  char dir[4];
  compass.getDirection(dir, (int)heading);
  direction = String(dir);

  // TSL2591
  sensors_event_t event;
  tsl.getEvent(&event);
  full = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
  ir   = tsl.getLuminosity(TSL2591_INFRARED);
  vis  = tsl.getLuminosity(TSL2591_VISIBLE);

  // MLX90614
  float objectTemp = mlx.readObjectTempC();
  float ambientTemp = mlx.readAmbientTempC();

  // Analog UV
  int rawUV = analogRead(UV_SENSOR_PIN);
  float uvVoltage = rawUV * (3.3 / 4095.0);
  float uvIndexAnalog = uvVoltage * 15.0;

  // VEML6075
  float uva = uv.readUVA();
  float uvb = uv.readUVB();
  float uvi = uv.readUVI();

  // BME280
  float tempBME = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();

  // Mic
  int micRaw = analogRead(MIC_PIN);

  // --- Serial Output ---
  Serial.print("Roll: "); Serial.print(roll, 2);
  Serial.print(" | Pitch: "); Serial.print(pitch, 2);
  Serial.print(" | Yaw: "); Serial.print(yaw, 2);
  Serial.print(" | Alt (m): "); Serial.print(altitude, 2);

  Serial.print(" | MagX: "); Serial.print(magX);
  Serial.print(" | MagY: "); Serial.print(magY);
  Serial.print(" | MagZ: "); Serial.print(magZ);
  Serial.print(" | Heading: "); Serial.print(heading, 2);
  Serial.print(" ("); Serial.print(direction); Serial.print(")");

  Serial.print(" | Lux: "); Serial.print(event.light, 2);
  Serial.print(" | Full: "); Serial.print(full);
  Serial.print(" | IR: "); Serial.print(ir);
  Serial.print(" | Visible: "); Serial.print(vis);

  Serial.print(" | IR Temp (C): "); Serial.print(objectTemp, 2);
  Serial.print(" | Ambient Temp (C): "); Serial.print(ambientTemp, 2);

  Serial.print(" | UV (Analog) ADC: "); Serial.print(rawUV);
  Serial.print(" | UV V: "); Serial.print(uvVoltage, 2);
  Serial.print(" | UV Index (Analog): "); Serial.print(uvIndexAnalog, 2);

  Serial.print(" | UVA: "); Serial.print(uva, 2);
  Serial.print(" | UVB: "); Serial.print(uvb, 2);
  Serial.print(" | UV Index (VEML): "); Serial.print(uvi, 2);

  Serial.print(" | Temp (BME280): "); Serial.print(tempBME, 2);
  Serial.print(" | Pressure: "); Serial.print(pressure, 2);
  Serial.print(" | Humidity: "); Serial.print(humidity, 2);

  Serial.print(" | Mic Raw: "); Serial.println(micRaw);

  delay(200);
}

void readMPU6050() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  GyroX -= GyroErrorX; GyroY -= GyroErrorY; GyroZ -= GyroErrorZ;

  unsigned long currentTime = millis();
  elapsedTime = (currentTime - prevTime) / 1000.0;
  prevTime = currentTime;

  accAngleX = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 180 / PI - AccErrorX;
  accAngleY = atan(-AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 180 / PI - AccErrorY;
  gyroAngleX += GyroX * elapsedTime;
  gyroAngleY += GyroY * elapsedTime;
  yaw        += GyroZ * elapsedTime;

  roll  = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  accZ_g = AccZ - AccZError;
  float accZ_m_s2 = (accZ_g - 1.0) * 9.81;
  velocityZ += accZ_m_s2 * elapsedTime;
  altitude  += velocityZ * elapsedTime;
}

void calculate_IMU_error() {
  float AccX_sum = 0, AccY_sum = 0, AccZ_sum = 0;
  float GyroX_sum = 0, GyroY_sum = 0, GyroZ_sum = 0;

  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    float ax = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float ay = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float az = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccX_sum += ax; AccY_sum += ay; AccZ_sum += az;

    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    float gx = (Wire.read() << 8 | Wire.read()) / 131.0;
    float gy = (Wire.read() << 8 | Wire.read()) / 131.0;
    float gz = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroX_sum += gx; GyroY_sum += gy; GyroZ_sum += gz;

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
