/*!
   @file ACE128tiny.cpp

   @mainpage ACE128tiny Library

   @section intro_sec Introduction

   This is a library for the ACE128tiny absolute encoder module which is a
   Bourns EAW Absolute Contacting Encoder (ACE) with an ATtiny841 backpack

   This device uses I2C to communicate, 2 pins are required to interface

   Some this code library inspired by work by Adafruit, and it depends
   on the Adafruit BusIO Library for cross-device support. Adafruit rock!

   @section author Author

   Written by Alastair Young DBA Red Hunter

   @section license License

   MIT license, all text above must be included in any redistribution.
   See the license file at the root of this github repo for full license text.
*/
#include "ACE128tiny.h"
#include <Adafruit_I2CDevice.h>

// Constructor
ACE128tiny::ACE128tiny() {}

// public methods
/*!
   @brief Starts I2C connection
   @param i2cAddr I2C address to use, ACE128TINY_I2C_DEFAULT by default
   @param wire The I2C interface to use, defaults to Wire
   @return Returns true if successful
*/
bool ACE128tiny::begin(uint8_t i2cAddr, TwoWire *wire) {
  _wire = wire; // save this in case we change address
  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(i2cAddr, wire);

  if (!i2c_dev->begin()) {
    return false;
  }
  return true;
}

// common getters
// -32767 - + 32767  0x8000 = ERROR
int16_t ACE128tiny::mpos() {
  return read16(ACE128TINY_REGISTER_MPOS);
}

//      0 - + 127      0xFF = ERROR
uint8_t ACE128tiny::upos() {
  return read8(ACE128TINY_REGISTER_UPOS);
}

// -64 - + 63       0x80 = ERROR
int8_t ACE128tiny::pos() {
  return (int8_t)read8(ACE128TINY_REGISTER_POS);
}

// common setters
// sets current position to multiturn value - also changes zero
void ACE128tiny::setMpos(int16_t mPos) {
  write16(ACE128TINY_REGISTER_MPOS, mPos);
}

// set counter-clockwise operation
void ACE128tiny::setReverse(bool reverse) {
  write8(ACE128TINY_REGISTER_REV, reverse ? 1 : 0);
}

// uncommon getters
// returns logical zero position
uint8_t ACE128tiny::getZero() {
  return read8(ACE128TINY_REGISTER_ZERO);
}

// returns raw mechanical position
uint8_t ACE128tiny::rawPos() {
  return read8(ACE128TINY_REGISTER_RAW);
}

// returns gray code inputs
uint8_t ACE128tiny::acePins() {
  return read8(ACE128TINY_REGISTER_PIN);
}

// get rotation true=anticlockwise
bool ACE128tiny::getReverse() {
  return read8(ACE128TINY_REGISTER_REV);
}

// get internal multiturn offset
int16_t ACE128tiny::getMof() {
  return read16(ACE128TINY_REGISTER_MOF);
}

// uncommon setters
// sets logical zero position for setting to a specific spot in single turn setups
void ACE128tiny::setZero(uint8_t rawPos) {
  write8(ACE128TINY_REGISTER_ZERO, rawPos);
}

// sets I2C address
void ACE128tiny::setAddr(uint8_t i2cAddr) {
  write8(ACE128TINY_REGISTER_I2C, i2cAddr);
  delete this->i2c_dev; // remove old interface
  this->i2c_dev = new Adafruit_I2CDevice(i2cAddr, this->_wire);
}

// rw helpers borrowed from Adafruit
uint8_t ACE128tiny::read8(uint8_t a) {
  uint8_t ret;
  i2c_dev->write_then_read(&a, 1, &ret, 1, true);
  return ret;
}

void ACE128tiny::write8(uint8_t a, uint8_t d) {
  i2c_dev->write(&d, 1, true, &a, 1);
}

uint16_t ACE128tiny::read16(uint8_t a) {
  uint8_t retbuf[2];
  uint16_t ret;

  i2c_dev->write_then_read(&a, 1, retbuf, 2, true);
  ret = retbuf[1] | (retbuf[0] << 8);
  return ret;
}


void ACE128tiny::write16(uint8_t a, uint16_t d) {
  uint8_t buf[2] = { highByte(d), lowByte(d) };
  i2c_dev->write(&buf[0], 2, true, &a, 1);
}
