#define OV2640_MINI_2MP_PLUS
#include "memorysaver.h"
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>

#define CS_PIN 5  // Chip select for ArduCAM

ArduCAM myCAM(OV2640, CS_PIN);

void setup() {
  uint8_t vid, pid;

  Serial.begin(115200);
  delay(1000);
  Serial.println("ArduCAM Start");

  // Initialize SPI
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  // Reset the camera
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);

  // Test if ArduCAM is connected
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26) || (pid != 0x42)) {
    Serial.print("Failed to detect ArduCAM! VID: ");
    Serial.print(vid, HEX);
    Serial.print(" PID: ");
    Serial.println(pid, HEX);
    while (1);
  } else {
    Serial.println("ArduCAM detected.");
  }

  // Initialize OV2640 camera
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240); // You can change resolution
  delay(1000);
}

void loop() {
  Serial.println("Capturing image...");

  // Clear FIFO
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();

  // Start capture
  myCAM.start_capture();
  Serial.println("Waiting for capture...");
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

  Serial.println("Capture done.");

  // Read length of image
  uint32_t len = myCAM.read_fifo_length();
  if (len >= 0x7FFFF) {
    Serial.println("Image too large.");
    return;
  } else if (len == 0 ) {
    Serial.println("Image capture failed.");
    return;
  }

  Serial.print("Image size: ");
  Serial.println(len);

  Serial.println("Sending image data:");

  // Send start marker
  Serial.write(0xFF);
  Serial.write(0xD8);

  // Read and send image bytes
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  for (uint32_t i = 0; i < len; i++) {
    uint8_t b = SPI.transfer(0x00);
    Serial.write(b);
  }
  myCAM.CS_HIGH();

  // Send end marker
  Serial.write(0xFF);
  Serial.write(0xD9);

  Serial.println("\nImage sent.");
  delay(5000); // Wait before taking next picture
}
