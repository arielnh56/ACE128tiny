#include <WireS.h>
#include <EEPROM.h>

#include "ACE128map12345678.h" // mapping for pin order 12345678
uint8_t *_map = (uint8_t*)encoderMap_12345678;


#define ACE_EEPROM_ZERO 0  // 1 byte
#define ACE_EEPROM_MULTITURN 1  //2 bytes
#define ACE_EEPROM_REVERSE 3  //1 bytes

const uint8_t pinOrder[8] = { 10, 9, 8, 7, 3, 2, 1, 0 };

uint8_t _zero;                 // raw position of logical zero
int16_t _mpos;                 // multiturn offset
int16_t _multiturn;                 // multiturn offset
int8_t _lastpos;               // last upos
uint8_t _acePins;              // last acePins
uint8_t _rawPos;               // last rawPos
int8_t _pos;               // last pos
uint8_t _upos;               // last upos
boolean _reverse;

void setup() {
  uint8_t i;
  for (i = 0; i <= 7; i++) {
    pinMode(pinOrder[i], INPUT_PULLUP);
  }

  EEPROM.get(ACE_EEPROM_MULTITURN, _mpos);
  EEPROM.get(ACE_EEPROM_ZERO, _zero);
  EEPROM.get(ACE_EEPROM_REVERSE, _reverse);
  _lastpos = 0;
  Wire.begin(0x20);
  Wire.onRequest(requestEvent);
}

void loop() {
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
  _lastpos = _pos;
  EEPROM.put(ACE_EEPROM_MULTITURN, _mpos);
  _multiturn = _mpos + _pos;

}

// returns the current value on the input pins
// Used internally, but exposed to help verify mapping tables
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
  Wire.write(_rawPos);
}
