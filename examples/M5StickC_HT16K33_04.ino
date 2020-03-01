// 1 x Adafruit 16x8 LED Matrix FeatherWing
// display Japanese font
#include <M5StickC.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <misakiUTF16.h>

Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

void setup() {
  M5.begin();
  Wire.begin(32, 33);
  Serial.begin(9600);
  Serial.println("16x8 + 8x8 LED Matrix Test");  
  matrix.begin(0x70);  // pass in the address
}

void loop() {
  uint8_t font[8];  
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
  //
  // start         0x70
  //               oooooooooooooooo ^ Hello World
  //              01             16   17
  // end
  // Hello world v oooooooooooooooo
  // - (num of chars) x 8
  // 
  for (int8_t x=17; x>=-8; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
    getFontData(font, "あ");
    matrix.drawBitmap(x,0,font,8,8,1);
    matrix.writeDisplay();
    delay(100);
  }
  matrix.setRotation(0);
}
