// defines shared by firmware and software
// registers - invalid writes are ignored
#define ACE128TINY_REGISTER_VER  0  // RO Firmware revision number. Revision 1 is current for the remainder of this list
#define ACE128TINY_REGISTER_MPOS 1  // RW multiturn position uint16_t -32767 - +32767 0x8000=ERROR
#define ACE128TINY_REGISTER_UPOS 2  // RO unsigned position uint8_t   0 - 127 0xFF=ERROR
#define ACE128TINY_REGISTER_POS  3  // RO signed position   int8_t  -64 - +63 0x80=ERROR
#define ACE128TINY_REGISTER_ZERO 4  // RW logical zero uint8_t (also writeable by updating mpos, will offset mpos when changed)
#define ACE128TINY_REGISTER_REV  5  // RW reverse 0=clockwise rising, 1=clockwise falling
#define ACE128TINY_REGISTER_RAW  6  // RO raw position (map table output) uint8_t for debug
#define ACE128TINY_REGISTER_PIN  7  // RO ACE pin output (map table input) uint8_t for debug
#define ACE128TINY_REGISTER_I2C  8  // RW I2C address. uint8_t 0x08 - 0x77. Default is 0x56
#define ACE128TINY_REGISTER_MOF  9  // RO Stored multiturn offset int16_t for debug
#define ACE128TINY_REGISTER_MAX  9  // Number of last register
#define ACE128TINY_I2C_DEFAULT 0x56
#define ACE128TINY_I2C_MIN 0x08
#define ACE128TINY_I2C_MAX 0x77
