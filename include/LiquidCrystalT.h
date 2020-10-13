#pragma once

#include <Arduino.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define NO_PIN 0xFF

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4 = NO_PIN, const uint8_t _D5 = NO_PIN, const uint8_t _D6 = NO_PIN, const uint8_t _D7 = NO_PIN, const uint8_t RW = NO_PIN>
class LiquidCrystalT : public Print {
public:
  LiquidCrystalT();

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setRowOffsets(int row1, int row2, int row3, int row4);
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);
  virtual size_t write(uint8_t);
  void command(uint8_t);

  using Print::write;

private:
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void write8bits(uint8_t);
  void pulseEnable();

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _numlines;
  uint8_t _row_offsets[4];
};

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::LiquidCrystalT() {
  if ((_D4 == NO_PIN) || (_D5 == NO_PIN) | (_D6 == NO_PIN) || (_D7 == NO_PIN))
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

  begin(16, 1);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  pinMode(RS, OUTPUT);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  if (RW != NO_PIN) {
    pinMode(RW, OUTPUT);
  }
  pinMode(ENABLE, OUTPUT);

  pinMode(_D0, OUTPUT);
  pinMode(_D1, OUTPUT);
  pinMode(_D2, OUTPUT);
  pinMode(_D3, OUTPUT);
  if (_displayfunction & LCD_8BITMODE) {
    pinMode(_D4, OUTPUT);
    pinMode(_D5, OUTPUT);
    pinMode(_D6, OUTPUT);
    pinMode(_D7, OUTPUT);
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  delayMicroseconds(50000);
  // Now we pull both RS and R/W low to begin commands
  digitalWrite(RS, LOW);
  digitalWrite(ENABLE, LOW);
  if (RW != NO_PIN) {
    digitalWrite(RW, LOW);
  }

  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03);
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02);
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::setRowOffsets(int row0, int row1, int row2, int row3) {
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::clear() {
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::home() {
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::setCursor(uint8_t col, uint8_t row) {
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);

  if (row >= max_lines) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if (row >= _numlines) {
    row = _numlines - 1;    // we count rows starting w/0
  }

  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::leftToRight() {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::rightToLeft() {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::autoscroll() {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::noAutoscroll() {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i = 0; i < 8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
inline void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::command(uint8_t value) {
  send(value, LOW);
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
inline size_t LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::write(uint8_t value) {
  send(value, HIGH);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::send(uint8_t value, uint8_t mode) {
  digitalWrite(RS, mode);

  // if there is a RW pin indicated, set it low to Write
  if (RW != NO_PIN) {
    digitalWrite(RW, LOW);
  }

  if (_displayfunction & LCD_8BITMODE) {
    write8bits(value);
  } else {
    write4bits(value >> 4);
    write4bits(value);
  }
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::pulseEnable() {
  digitalWrite(ENABLE, LOW);
  delayMicroseconds(1);
  digitalWrite(ENABLE, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450ns
  digitalWrite(ENABLE, LOW);
  delayMicroseconds(100);   // commands need > 37us to settle
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::write4bits(uint8_t value) {
  digitalWrite(_D0, value & 0x01);
  digitalWrite(_D1, (value >> 1) & 0x01);
  digitalWrite(_D2, (value >> 2) & 0x01);
  digitalWrite(_D3, (value >> 3) & 0x01);

  pulseEnable();
}

template<const uint8_t RS, const uint8_t ENABLE, const uint8_t _D0, const uint8_t _D1, const uint8_t _D2, const uint8_t _D3,
  const uint8_t _D4, const uint8_t _D5, const uint8_t _D6, const uint8_t _D7, const uint8_t RW>
void LiquidCrystalT<RS, ENABLE, _D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, RW>::write8bits(uint8_t value) {
  digitalWrite(_D0, value & 0x01);
  digitalWrite(_D1, (value >> 1) & 0x01);
  digitalWrite(_D2, (value >> 2) & 0x01);
  digitalWrite(_D3, (value >> 3) & 0x01);
  digitalWrite(_D4, (value >> 4) & 0x01);
  digitalWrite(_D5, (value >> 5) & 0x01);
  digitalWrite(_D6, (value >> 6) & 0x01);
  digitalWrite(_D7, (value >> 7) & 0x01);

  pulseEnable();
}
