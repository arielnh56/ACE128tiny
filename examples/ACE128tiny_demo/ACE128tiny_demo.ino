#define REGISTER_MPOS 0
#define REGISTER_UPOS 1
#define REGISTER_POS  2
#define REGISTER_I2C  3
#define REGISTER_VER  4



#include <Wire.h>

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

void loop() {
  Wire.beginTransmission(0x20);
  Wire.write(REGISTER_MPOS);
  Wire.endTransmission();
  Wire.requestFrom(0x20, 2);    // request 6 bytes from slave device #8
  int16_t mpos = Wire.read() + (Wire.read() << 8);
  Serial.print(mpos, DEC);         // print the character
  Serial.print("   ");

  Wire.beginTransmission(0x20);
  Wire.write(REGISTER_UPOS);
  Wire.endTransmission();
  Wire.requestFrom(0x20, 1);    // request 6 bytes from slave device #8
  uint8_t upos = Wire.read();
  Serial.print(upos, DEC);         // print the character
  Serial.print("   ");
  
  Wire.beginTransmission(0x20);
  Wire.write(REGISTER_POS);
  Wire.endTransmission();
  Wire.requestFrom(0x20, 1);    // request 6 bytes from slave device #8
  int8_t pos = Wire.read();
  Serial.print(pos, DEC);         // print the character
  Serial.println("   ");
  
  delay(50);
}
