#include <Wire.h>

void setup() {
  Serial.begin(9600);  // Start serial communication
  Wire.begin();        // Start I²C
  Serial.println("Scanning...");

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println();
    }
  }
  Serial.println("Scan complete.");
}

void loop() {
  // Empty loop
}
