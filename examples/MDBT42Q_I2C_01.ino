#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

void setup() {
  Wire.setPins(4, 5);  // 4:SDA 5:SCL
  Wire.begin();
  matrix.begin(0x70);
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
}

void loop() {
  //
  // start         0x70
  //               oooooooo ^ Hello World
  //              01      8   9
  // end
  // Hello world v oooooooo
  // - (num of chars) x 6
  // 
  for (int8_t x=9; x>=-66; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
    matrix.print("Hello World");
    matrix.writeDisplay();
    delay(100);
  }
//  matrix.setRotation(0);
}
