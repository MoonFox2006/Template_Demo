#include <Arduino.h>
#include "LiquidCrystalT.h"

const uint8_t rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 9;

LiquidCrystalT<rs, en, d4, d5, d6, d7> lcd;

void setup() {
  pinMode(bl, OUTPUT);
  analogWrite(bl, 127);

  lcd.begin(16, 2);
  lcd.print(F("Hello, World!"));
  delay(2000);
}

void loop() {
  lcd.home();
  lcd.print(F("Uptime "));
  lcd.print(millis() / 1000);
  lcd.print(F(" sec."));
  delay(1000);
}
