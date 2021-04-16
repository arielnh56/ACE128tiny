#include <Wire.h>

#include <EEPROM.h>

#include "ACE128map12345678.h" // mapping for pin order 12345678
uint8_t *_map = (uint8_t*)encoderMap_12345678;

#define EEPROM_BASE          0                       // Base address in EEPROM
#define EEPROM_ZERO        EEPROM_BASE + 1       // 1 byte
#define EEPROM_MULTITURN   EEPROM_ZERO + 1       // 2 bytes
#define EEPROM_REVERSE     EEPROM_MULTITURN + 2  // 1 byte
#define EEPOROM_I2CADDR    EEPROM_REVERSE + 1    // 1 byte
#define EEPROM_DEFAULT 0x20

#define REGISTER_MPOS 0
#define REGISTER_UPOS 1
#define REGISTER_POS  2
#define REGISTER_I2C  3
#define REGISTER_VER  4
#define REGISTER_MAX  4

#define FIRMWARE_REV  0
 
const uint8_t pinOrder[8] = { 10, 9, 8, 7, 3, 2, 1, 0 };

uint8_t _zero;          // raw position of logical zero
int16_t _mpos;          // multiturn offset
int16_t _multiturn;     // multiturn offset
int8_t  _lastpos;       // last upos
uint8_t _acePins;       // last acePins
uint8_t _rawPos;        // last rawPos
int8_t  _pos;           // last pos
uint8_t _upos;          // last upos
boolean _reverse;       // reverse
uint8_t _i2cAddr;       // I2C address 
uint8_t _register;      // pointer to active register

void setup() {
  uint8_t i;
  for (i = 0; i <= 7; i++) {
    pinMode(pinOrder[i], INPUT_PULLUP);
  }

  EEPROM.get(EEPROM_MULTITURN, _mpos);
  EEPROM.get(EEPROM_ZERO, _zero);
  EEPROM.get(EEPROM_REVERSE, _reverse);
  _acePins = acePins();
  _lastpos = pgm_read_byte(_map + _acePins);
  _multiturn = _mpos + _lastpos;
  Wire.begin(0x20);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  _reverse = 0;
  _mpos = 0;
  _zero = 0;
}

void loop() {
  // change this to interrupt driven and sleep
  _acePins = acePins();
  _rawPos = pgm_read_byte(_map + _acePins);
  _pos = pos();
  _upos = upos();

  // mpos rollover check
  if ((int16_t)_lastpos - (int16_t)_pos > 0x40)    // more than half a turn smaller - we rolled up
  {
    _mpos += 0x80;
  }
  else if ((int16_t)_pos - (int16_t)_lastpos > 0x40)   // more than half a turn bigger - we rolled down
  {
    _mpos -= 0x80;
  }
  if (_lastpos != _pos) {
    EEPROM.put(EEPROM_MULTITURN, _mpos); // only writes if it changed
    _lastpos = _pos;
    _multiturn = _mpos + _pos;
  }
}

// returns the current value on the input pins
// If you ever get a 255 from a mapping table, something is wrong
uint8_t acePins(void)
{
  uint8_t returnVal = 0x00;
  for (uint8_t i = 0; i <= 7; i++) {
    returnVal |= digitalRead(pinOrder[i]) << i;
  }
  return returnVal;
}

// returns signed position -64 - +63
int8_t pos(void) {
  int8_t pos = _rawPos - _zero;   // adjust for logical zero
  if (_reverse) pos *= -1;    // reverse direction
  // 7bit signed numbers need to copy their neg bit to the 8bit position
  if ( pos & 0x40 ) { // check for 7bit neg bit
    pos |= 0x80; // set 8bit neg bit
  } else {
    pos &= 0x7F; // clear 8bit neg bit
  }
  return (pos);
}

// returns unsigned position 0 - 127
uint8_t upos(void) {
  int8_t pos = _rawPos - _zero;             // adjust for logical zero
  if (_reverse) pos *= -1;  // reverse direction

  pos &= 0x7F; // clear the 8bit neg bit
  return (pos);
}

void requestEvent() {
  switch(_register) {
    case REGISTER_MPOS :
      Wire.write((byte*)&_multiturn,2);
      break;
    case REGISTER_UPOS :
      Wire.write(upos());
      break;
    case REGISTER_POS :
      Wire.write(pos());
      break;
    case REGISTER_I2C :
      Wire.write(0x20);
      break;
  }
}

void receiveEvent(int howMany) {
  uint8_t inByte;
  // first byte sets the register
  if (Wire.available() == 0) return; // no data
  inByte = Wire.read();
  if (inByte > REGISTER_MAX) goto drain; // invalid register
  _register = inByte;
  if (Wire.available() > 0) { // we are writing
    switch(_register) {
      case REGISTER_MPOS :
        break;
      case REGISTER_I2C :
        break;
    }
  }
  drain:
  while (Wire.available() > 0) Wire.read(); 
}
