// Adafruit LED Matrix backpacks
#include <M5StickC.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
Adafruit_8x8matrix matrix1 = Adafruit_8x8matrix();

void setup() {
  M5.begin();
  Wire.begin(32, 33);
  Serial.begin(9600);
  Serial.println("8x8 LED Matrix Test");  
  matrix.begin(0x71);  // pass in the address
  matrix1.begin(0x72);  // pass in the address
}

void loop() {
  matrix.setTextSize(1);
  matrix1.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix1.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix1.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix1.setRotation(1);
  //
  // start         0x71     0x70
  //                       oooooooo ^ Hello World
  //                      01      8   9
  //               oooooooo
  //              01      8
  // end
  //                       oooooooo ^
  // Hello world v oooooooo
  //                       - (num of chars) x 6 - 8
  // - (num of chars) x 6
  // 
  for (int8_t x=9; x>=-74; x--) {
    matrix.clear();
    matrix1.clear();
    matrix.setCursor(x,0);
    matrix1.setCursor(x+8,0);
    matrix.print("Hello World");
    matrix.writeDisplay();
    matrix1.print("Hello World");
    matrix1.writeDisplay();
    delay(100);
  }
  matrix.setRotation(0);
  matrix1.setRotation(0);
}
