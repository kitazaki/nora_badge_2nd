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
  uint8_t buf[8];
  char *str="日本後のテストです";
  for (int8_t x=17; x>=-72; x--) {
    char *ptr = str;
    uint16_t n = 0;
    matrix.clear();
    matrix.setCursor(x,0);
    while(*ptr){
      ptr = getFontData(buf,ptr,true);
      if(!ptr)
        break;
      matrix.drawBitmap(x+n,0,buf,8,8,1);
      n+=8;
    }
    matrix.writeDisplay();
    delay(100);
  }
  matrix.setRotation(0);
}
