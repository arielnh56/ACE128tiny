#include <ACE128tiny.h>

#include <Wire.h>

ACE128tiny knob;
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

const int setMposPin = 12;
const int setRevPin = 11;
const int setFwdPin = 10;
const int setI2C42Pin = 9;
const int setI2C56Pin = 8;
const int setZero44Pin = 7;
const int setZero28Pin = 6;

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  knob.begin(0x56);
  pinMode(setMposPin, INPUT_PULLUP);
  pinMode(setRevPin, INPUT_PULLUP);
  pinMode(setFwdPin, INPUT_PULLUP);
  pinMode(setI2C42Pin, INPUT_PULLUP);
  pinMode(setI2C56Pin, INPUT_PULLUP);
  pinMode(setZero44Pin, INPUT_PULLUP);
  pinMode(setZero28Pin, INPUT_PULLUP);
}

uint8_t oldPins = 0;
uint8_t acePins = 0;

void loop() {
  while (acePins == oldPins) {
    if (digitalRead(setMposPin) == LOW ) {
      Serial.println("Setting mpos to 537");
      knob.setMpos(537);
      while (  digitalRead(setMposPin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    if (digitalRead(setRevPin) == LOW ) {
      Serial.println("Setting reverse true");
      knob.setReverse(true);
      while (  digitalRead(setRevPin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    if (digitalRead(setFwdPin) == LOW ) {
      Serial.println("Setting reverse false");
      knob.setReverse(false);
      while (  digitalRead(setFwdPin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    if (digitalRead(setI2C42Pin) == LOW ) {
      Serial.println("Setting i2c 0x42");
      knob.setAddr(0x42);
      while (  digitalRead(setI2C42Pin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    if (digitalRead(setI2C56Pin) == LOW ) {
      Serial.println("Setting i2c 0x56");
      knob.setAddr(0x56);
      while (  digitalRead(setI2C56Pin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    if (digitalRead(setZero44Pin) == LOW ) {
      Serial.println("Setting zero to 44");
      knob.setZero(44);
      while (  digitalRead(setZero44Pin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    if (digitalRead(setZero28Pin) == LOW ) {
      Serial.println("Setting zero to 28");
      knob.setZero(28);
      while (  digitalRead(setZero28Pin) == LOW ) delay(1000);
      oldPins = 0xff;
    }

    acePins = knob.acePins();
    delay(5);
  }
  oldPins = acePins;




  char binaryString[8];
  sprintf(binaryString, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(acePins));
  Serial.println();
  Serial.print("pins      ");
  Serial.print(binaryString);    Serial.print("   ");
  Serial.println(acePins, DEC);
  int16_t mpos = knob.mpos();
  Serial.print("mpos      ");
  Serial.println(mpos, DEC);
  int16_t mof = knob.getMof();
  Serial.print("mof      ");
  Serial.println(mof, DEC);
  uint8_t upos = knob.upos();
  Serial.print("upos      ");
  Serial.println(upos, DEC);
  int8_t pos = knob.pos();
  Serial.print("pos       ");
  Serial.println(pos, DEC);
  uint8_t zero = knob.getZero();
  Serial.print("zero      ");
  Serial.println(zero, DEC);
  uint8_t raw = knob.rawPos();
  Serial.print("raw       ");
  Serial.println(raw, DEC);
  int8_t rev = knob.getReverse();
  Serial.print("reverse   ");
  Serial.println(rev ? "true" : "false");

}
