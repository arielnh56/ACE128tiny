
#include "ACE128tiny.h"

#include <Arduino.h>
#include <Wire.h>

#include <EEPROM.h>

#include "ACE128map12345678.h" // mapping for pin order 12345678
uint8_t *_map = (uint8_t*)encoderMap_12345678;

#define ACE128TINY_EEPROM_BASE        0                                // Base address in EEPROM
#define ACE128TINY_EEPROM_ZERO        ACE128TINY_EEPROM_BASE + 1       // 1 byte
#define ACE128TINY_EEPROM_MULTITURN   ACE128TINY_EEPROM_ZERO + 1       // 2 bytes
#define ACE128TINY_EEPROM_REVERSE     ACE128TINY_EEPROM_MULTITURN + 2  // 1 byte
#define ACE128TINY_EEPROM_I2CADDR     ACE128TINY_EEPROM_REVERSE + 1    // 1 byte
// 5 bytes total
#define ACE128TINY_FIRMWARE_REV  1   // version number for future backward compatibility

const uint8_t pinOrder[8] = { 10, 9, 8, 7, 3, 2, 1, 0 };  // may change by board design

uint8_t _zero;          // raw position of logical zero
int16_t _mpos;          // multiturn offset
int16_t _multiturn;     // multiturn offset
int8_t  _lastpos;       // last upos
uint8_t _acePins;       // last acePins
uint8_t _rawPos;        // last rawPos
int8_t  _pos;           // last pos
uint8_t _upos;          // last upos
uint8_t _reverse;       // reverse - really a boolean but we want to force a 1 bit behavior
uint8_t _i2cAddr;       // I2C address
uint8_t _register;      // pointer to active register

void setup() {
  for (uint8_t i = 0; i <= 7; i++) {
    pinMode(pinOrder[i], INPUT_PULLUP);
  }

  // read EEPROM. Make sure we get sensible values in case we didn't write it...
  EEPROM.get(ACE128TINY_EEPROM_ZERO, _zero);
  _zero &= 0x7f; // 7 bit value only
  EEPROM.get(ACE128TINY_EEPROM_MULTITURN, _mpos);
  _mpos &= 0xFF80; // lower 7 bits should be clear
  EEPROM.get(ACE128TINY_EEPROM_REVERSE, _reverse);
  _reverse &= 0x01; // 1 bit only
  EEPROM.get(ACE128TINY_EEPROM_I2CADDR, _i2cAddr);
  if ( _i2cAddr < ACE128TINY_I2C_MIN || _i2cAddr > ACE128TINY_I2C_MAX ) _i2cAddr = ACE128TINY_I2C_DEFAULT;

  // where are we?
  _acePins = acePins();
  _rawPos = pgm_read_byte(_map + _acePins);
  if (_reverse) _rawPos = 0x7F - _rawPos;
  _pos = pos();
  _lastpos = _pos;
  _upos = upos();
  _multiturn = _mpos + _pos;

  // set up I2C
  Wire.begin(_i2cAddr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop() {
  // change this to interrupt driven and sleep
  _acePins = acePins();
  _rawPos = pgm_read_byte(_map + _acePins);
  if (_reverse) _rawPos = 0x7F - _rawPos;
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
  EEPROM.put(ACE128TINY_EEPROM_MULTITURN, _mpos);
  _lastpos = _pos;
  _multiturn = _mpos + _pos;
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
  return raw2pos(_rawPos);
}

// returns unsigned position 0 - 127
uint8_t upos(void) {
  int8_t pos = _rawPos - _zero;             // adjust for logical zero
  pos &= 0x7F; // clear the 8bit neg bit
  return (pos);
}

void requestEvent() {
  switch (_register) {
    case ACE128TINY_REGISTER_VER :
      Wire.write(ACE128TINY_FIRMWARE_REV);
      break;
    case ACE128TINY_REGISTER_MPOS :
      Wire.write(highByte(_multiturn));
      Wire.write(lowByte(_multiturn));
      break;
    case ACE128TINY_REGISTER_UPOS :
      Wire.write(_upos);
      break;
    case ACE128TINY_REGISTER_POS :
      Wire.write(_pos);
      break;
    case ACE128TINY_REGISTER_ZERO :
      Wire.write(_zero);
      break;
    case ACE128TINY_REGISTER_REV :
      Wire.write(_reverse ? 1 : 0);
      break;
    case ACE128TINY_REGISTER_RAW :
      Wire.write(_rawPos);
      break;
    case ACE128TINY_REGISTER_PIN :
      Wire.write(_acePins);
      break;
    case ACE128TINY_REGISTER_I2C :
      Wire.write(_i2cAddr);
      break;
    case ACE128TINY_REGISTER_MOF :
      Wire.write(highByte(_mpos));
      Wire.write(lowByte(_mpos));
      break;
  }
}

void receiveEvent(int howMany) {
  uint8_t inByte;
  // first byte sets the register
  if (Wire.available() == 0) return; // no data
  inByte = Wire.read();
  if (inByte > ACE128TINY_REGISTER_MAX) goto drain; // invalid register
  _register = inByte;
  if (Wire.available() > 0) { // we are writing
    switch (_register) {
      case ACE128TINY_REGISTER_MPOS :
        if (Wire.available() >= 2) {
          int16_t mPos = (uint16_t)Wire.read() << 8 | (uint16_t)Wire.read();
          setZero(_rawPos - (uint8_t)(mPos & 0x7f));  // mask to 7bit
          _lastpos = raw2pos(_rawPos);
          _mpos = (int16_t)((mPos - _lastpos) & 0xFF80);          // mask higher 9 bits
          EEPROM.put(ACE128TINY_EEPROM_MULTITURN, _mpos);
        }
        break;
      case ACE128TINY_REGISTER_ZERO :
        if (Wire.available()) {
          setZero(Wire.read() & 0x7F);
        }
        break;
      case ACE128TINY_REGISTER_REV :
        if (Wire.available()) {
          inByte = Wire.read() & 0x01;
          _reverse = inByte;
          EEPROM.put(ACE128TINY_EEPROM_REVERSE, inByte);
        }
        break;
      case ACE128TINY_REGISTER_I2C :
        if (Wire.available()) {
          inByte = Wire.read();
          if (ACE128TINY_I2C_MIN <= inByte && inByte <= ACE128TINY_I2C_MAX) {
            _i2cAddr = inByte;
            EEPROM.put(ACE128TINY_EEPROM_I2CADDR, _i2cAddr);
            Wire.begin(_i2cAddr);
          }
        }
        break;
    }
  }
drain:
  while (Wire.available() > 0) Wire.read();
}

// sets logical zero position
void setZero(uint8_t rawPos)
{
  _zero = rawPos & 0x7f;  // mask to 7bit
  EEPROM.put(ACE128TINY_EEPROM_ZERO, _zero);
  _pos = pos();
  _upos = upos();
  _lastpos = _pos;
  _multiturn = _mpos + _pos;
}

int8_t raw2pos(int8_t pos) {
  pos -= _zero;   // adjust for logical zero
  // 7bit signed numbers need to copy their neg bit to the 8bit position
  if ( pos & 0x40 ) { // check for 7bit neg bit
    pos |= 0x80; // set 8bit neg bit
  } else {
    pos &= 0x7F; // clear 8bit neg bit
  }
  return (pos);
}
