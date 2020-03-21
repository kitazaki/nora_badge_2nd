#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

class aitendo_KLED1608K33D_8x16matrix : public Adafruit_LEDBackpack, public Adafruit_GFX {
 public:
  aitendo_KLED1608K33D_8x16matrix(void);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};

aitendo_KLED1608K33D_8x16matrix::aitendo_KLED1608K33D_8x16matrix(void) : Adafruit_GFX(16, 8) {
}

void aitendo_KLED1608K33D_8x16matrix::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((y < 0) || (x < 0)) return;
  if ((getRotation() % 2 == 0) && ((x >= 16) || (y >= 8))) return;
  if ((getRotation() % 2 == 1) && ((y >= 16) || (x >= 8))) return;

  switch (getRotation()) {
  case 0:
    if (x >= 8) {
      x -= 8;
      y += 8; 
    }
    break;
  case 1:
    y = 16 - y - 1;
    if(y >= 8) {
      y -= 8;
      x += 8;
    }
    _swap_int16_t(x, y);
    break;
  case 2:
    x = 16 - x - 1;
    y = 8 - y - 1;
    if (x >= 8) {
      x -= 8;
      y += 8; 
    }
    break;
  case 3:
    x = 8 - x - 1;
    if(y >= 8) {
      y -= 8;
      x += 8;
    }
    _swap_int16_t(x, y);
    break;
  }

  if (color) {
    displaybuffer[x] |= 1 << y;
  } else {
    displaybuffer[x] &= ~(1 << y);
  }
}

aitendo_KLED1608K33D_8x16matrix matrix = aitendo_KLED1608K33D_8x16matrix();
aitendo_KLED1608K33D_8x16matrix matrix1 = aitendo_KLED1608K33D_8x16matrix();

void setup() {
  Wire.setPins(4, 5);  // 4:SDA 5:SCL
  Wire.begin();
  matrix.begin(0x70);
  matrix1.begin(0x71);
  matrix.setTextSize(1);
  matrix1.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix1.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix1.setTextColor(LED_ON);
  matrix.setRotation(0);
  matrix1.setRotation(0);
}

void loop() {
  //
  // start         0x71                    0x70
  //                               oooooooooooooooo ^ Hello World
  //                              01             1617
  //               oooooooooooooooo
  //              01             1617            3233
  // end
  //                               oooooooooooooooo ^
  // Hello world v oooooooooooooooo
  //                                       - (num of chars) x 6 - 16
  // - (num of chars) x 6
  // 
  for (int8_t x=17; x>=-82; x--) {
    matrix.clear();
    matrix1.clear();
    matrix.setCursor(x,0);
    matrix1.setCursor(x+16,0);
    matrix.print("Hello World");
    matrix.writeDisplay();
    matrix1.print("Hello World");
    matrix1.writeDisplay();
    delay(100);
  }
}
