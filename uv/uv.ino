#define UV_SENSOR_PIN 35  // Changed to D35 (GPIO 35)

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit resolution (0–4095)
}

void loop() {
  int rawValue = analogRead(UV_SENSOR_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  float uvIndex = voltage * 15.0;  // Rough estimation

  Serial.print("ADC: ");
  Serial.print(rawValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print(" V | UV Index: ");
  Serial.println(uvIndex, 2);

  delay(1000);
}
