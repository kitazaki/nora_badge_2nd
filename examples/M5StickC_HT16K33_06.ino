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
  M5.Lcd.setRotation(1);  // display size = 80x160, setRotation = 0:M5, 1:Power Btn, 2:up side down, 3:Btn
  Wire.begin(32, 33);
  Serial.begin(9600);
  Serial.println("16x8 + 8x8 LED Matrix Test");  
  matrix.begin(0x70);  // pass in the address
}

void loop() {
  M5.Lcd.setTextSize(3);
  
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
  
  for (int8_t x=60; x>=-90; x--) {
    M5.Lcd.setCursor(3*x, 10);
    writeKnj("日本語のテストです ");

    char *ptr = str;
    uint16_t n = 0;
    matrix.clear();
    matrix.setCursor(x,0);
    while(*ptr){
      ptr = getFontData(buf,ptr,true);
      if(!ptr)
        break;
      matrix.drawBitmap(x+17+n,0,buf,8,8,1);
      n+=8;
    }
    matrix.writeDisplay();
    delay(100);
  }
  matrix.setRotation(0);
}

void writeKnj(char *str) {
  int posX = M5.Lcd.getCursorX();
  int posY = M5.Lcd.getCursorY();
  uint8_t textsize = M5.Lcd.textsize;
  uint32_t textcolor = M5.Lcd.textcolor;
  uint32_t textbgcolor = M5.Lcd.textbgcolor;
   
  byte font[8];
   
  while( *str != 0x00 ){
    // 改行処理
    if( *str == '\n' ){
      // 改行
      posY += 8 * textsize;
      posX = M5.Lcd.getCursorX();
      str++;
      continue;
    }
 
    // 文字横幅
    int width = 8 * textsize;
    if( *str < 0x80 ){
      // 半角
      width = 4 * textsize;
    }
 
    // フォント取得
    str = getFontData( font, str );
 
    // 背景塗りつぶし
    M5.Lcd.fillRect(posX, posY, width, 8 * textsize, textbgcolor);
 
    // 取得フォントの描画
    for (uint8_t row = 0; row < 8; row++) {
      for (uint8_t col = 0; col < 8; col++) {
        if( (0x80 >> col) & font[row] ){
          int drawX = posX + col * textsize;
          int drawY = posY + row * textsize;
          if( textsize == 1 ){
            M5.Lcd.drawPixel(drawX, drawY, textcolor);
          } else {
            M5.Lcd.fillRect(drawX, drawY, textsize, textsize, textcolor);
          }
        }
      }
    }
 
    // 描画カーソルを進める
    posX += width;
  }
 
  // カーソルを更新
  M5.Lcd.setCursor(posX, posY);
}
