// Adafruit 16x8 LED Matrix FeatherWing
#include <M5StickC.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

void setup() {
  M5.begin();
  Wire.begin(32, 33);
  Serial.begin(9600);
  Serial.println("16x8 LED Matrix Test");  
  matrix.begin(0x70);  // pass in the address
}

void loop() {
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
  //
  // start
  //             v oooooooooooooooo ^ Hello World
  //            -1                 17
  // end
  // Hello world v oooooooooooooooo ^
  // - (num of chars) x 6
  // 
  for (int8_t x=17; x>=-65; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
    matrix.print("Hello World");
    matrix.writeDisplay();
    delay(100);
  }
  matrix.setRotation(0);
}
