#include <Wire.h>
#include <SPI.h>
#include <ArduCAM.h>
#include "memorysaver.h"

// Select your CS pin (we're using GPIO5)
const int CS = 5;

// Create camera object (OV2640 sensor, CS pin)
ArduCAM myCAM(OV2640, CS);

void setup() {
  uint8_t vid, pid;
  uint8_t temp;

  // Start serial for image output and debug
  Serial.begin(115200);
  delay(1000);
  Serial.println("ArduCAM Start");

  // Setup CS pin
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  // Begin SPI (SCK, MISO, MOSI, SS)
  SPI.begin(18, 19, 23, 5);  // SCK, MISO, MOSI, SS
  SPI.setFrequency(4000000); // 4 MHz

  // Check SPI
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println("SPI interface Error!");
    while (1);
  }

  // Check camera module ID
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26) || ((pid != 0x41) && (pid != 0x42))) {
    Serial.println("Can't find OV2640 module!");
    while (1);
  } else {
    Serial.println("OV2640 detected.");
  }

  // Init camera and set resolution
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);  // Change resolution if needed
  delay(1000);
  myCAM.clear_fifo_flag();

  // Take picture
  Serial.println("Capturing image...");
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
    delay(10);
  }

  Serial.println("Capture Done.");
  delay(100);

  // Read image length
  uint32_t len = myCAM.read_fifo_length();
  if (len >= 0x07FFFF || len == 0) {
    Serial.println("Invalid image length.");
    return;
  }

  // Start image transfer
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  Serial.println("IMG_START");  // Signal to Python script
  while (len--) {
    uint8_t b = SPI.transfer(0x00);
    Serial.write(b);
  }
  Serial.println("IMG_END");

  myCAM.CS_HIGH();
  Serial.println("Image sent.");
}

void loop() {
  // Nothing in loop
}
