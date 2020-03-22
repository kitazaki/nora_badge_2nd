// Nora BADGE 2nd

// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>
#include <string.h>
#include "NoraPeri.h"

// LED pin デバッグ用LED　Pin
#define LED_PIN   6  // 4:SDA 5:SCL 6:OUTPUT

// Import libraries (I2C 16x8 LED Matrix)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <misakiUTF16.h>  // for example

// for aitendo K-LED1608K33D 8x16matrix
// https://www.aitendo.com/product/16658
// for MJKDZ 8x16 KED matrix
// http://www.icstation.com/matrix-display-module-lattice-screen-ht16k33-driver-cascade-compatible-16x8-interface-p-14221.html
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

// init matrix1 (I2C address: 0x71) / matrix (I2C address: 0x70)
aitendo_KLED1608K33D_8x16matrix matrix = aitendo_KLED1608K33D_8x16matrix();
aitendo_KLED1608K33D_8x16matrix matrix1 = aitendo_KLED1608K33D_8x16matrix();

#define  SCREENS          4   // LED Matrixカスケード接続数
// B1144format Header
#define  ONE_CHAR_SIZE    11  //1文字分の文字データサイズ
#define  MAX_WRITE_LENGTH 16  //BLE Writeデータ最大サイズ

/* 左、右、固定スクロール対応 Y.Iida */
#define MODE_LEFT         0
#define MODE_RIGHT        1

#define MAXSCREENS 4  // 最大接続数
static uint8_t buf[8*MAXSCREENS];
uint8_t* MAX7219_getBuffer() {
  return buf;  
}

// create peripheral instance, see pinouts above
BLEPeripheral            blePeripheral;//        = BLEPeripheral();

// create service
BLEService               ledService           = BLEService(BLE_SERVICE_UUID);
BLEService               ledCharService       = BLEService(BLE_CHARACTERISTIC_SERVICE_UUID);

// create switch characteristic
unsigned char num                           = 16;     //
int MaxWriteLen                             = 16;     //Write受信サイズ
unsigned char LED_RecieveValue[16]          = "";     //Write受信データ
int           LED_RecieveLength             = 0;      //Write受信データサイズ
int           ConcateCount                  = 0;      //Write連結数(ヘッダ含む)
uint8_t       LED_payload[1408]             = {};     //ペイロード配列
unsigned int  B1122_CharLength              = 0;      //ペイロードの文字数
unsigned int  B1122_PayloadLines            = 0;      //ペーロード自体の数
unsigned int  MAX7219__CharLength           = 0;      //ペイロードの文字数(LEDボード入力用)
bool          isWritting                    = false;  //Writeイベント発生フラグ
int           margecount                    = 0;      //マージ用ループカウンター
int           PayloadLineCounter            = 0;      //ペイロードカウント
int           SurplusLength                 = 0;      //文字格納余り
int           WritePayloadSize              = 0;      //ペーロードのサイズ ( 11 * 文字数 )
uint8_t       MAX7219__payload[1024]        = {};     //文字配列(LED出力用)

uint8_t       display_Flash                 = 1;      //フラッシュ(点滅)設定
uint8_t       display_Marqee                = 0;      //マーキー設定
uint8_t       display_speed_mode            = 0x60;   //表示スピード＆モード

uint8_t       save_Flash                    = 0;      //フラッシュ(点滅)設定
uint8_t       save_Marqee                   = 1;      //マーキー設定
uint8_t       save_speed_mode               = 0x71;   //表示スピード＆モード

int           blanklength     = 32;


//16byteのデータ受信用Charasteristic
BLECharacteristic switchCharacteristic = BLECharacteristic(BLE_CHARACTERISTIC_DATA_UUID, BLERead | BLEWrite | BLENotify, num);

void setup() {
  Serial.begin(115200);  // BLE通信に必要
//  #if defined (__AVR_ATmega32U4__)
//    delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
//  #endif

  // for debug: set LED pin to output mode
  pinMode(LED_PIN, OUTPUT);

  const unsigned char noramanufacture[] = "Nora Hack Community";
  unsigned char noramanufacturerDataLength = 19;
  // set advertised local name and service UUID
  blePeripheral.setLocalName("LSLED");
  //blePeripheral.setLocalName("TEST_LSLED"); //デバッグ用
  //blePeripheral.setDeviceName("NORA_LED");
  blePeripheral.setDeviceName("LSLED");
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());
  blePeripheral.setAdvertisedServiceUuid(ledCharService.uuid());
  blePeripheral.setManufacturerData(noramanufacture, noramanufacturerDataLength);

  // add service and characteristic
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(ledCharService);
  blePeripheral.addAttribute(switchCharacteristic);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);

  //switchCharacteristic.setValue(1);

  // begin initialization
  blePeripheral.begin();

  int DefaultLength = 32;
  memset(MAX7219__payload, '¥0', sizeof(MAX7219__payload));
  uint8_t base[DefaultLength] = {0xc6, 0xe6, 0xe6, 0xf6, 0xde, 0xce, 0xce, 0xc6, 
                                 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 
                                 0xfe, 0xc6, 0xc6, 0xfe, 0xd8, 0xcc, 0xc6, 0xc6, 
                                 0xfe, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6
                                 };
  for(int basecount = 0; basecount < DefaultLength; basecount++){
    MAX7219__payload[basecount] = base[basecount];
  }
  MAX7219__CharLength = 4;

  // begin I2C 16x8 LED Matrix
  Wire.setPins(4, 5);  // 4:SDA 5:SCL
  Wire.begin();
  matrix.begin(0x70);
  matrix1.begin(0x71);
  // matrix.setTextSize(1);
  // matrix1.setTextSize(1);
  matrix.setBrightness(0);
  matrix1.setBrightness(0);
  matrix.setTextWrap(false);
  matrix1.setTextWrap(false);
  matrix.setTextColor(LED_ON);
  matrix1.setTextColor(LED_ON);
  matrix.setRotation(0);
  matrix1.setRotation(0);
}


void loop() {
  // poll peripheral
  blePeripheral.poll();

  if(isWritting == false){
    MAX7219Display(MAX7219__payload, MAX7219__CharLength, display_Flash, display_Marqee, display_speed_mode);
    // TestLEDMatrix();
    DebugLEDOnOff();
//    delay(100);
  }
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  //DebugLEDOnOff();
  //delay(500);
  Serial.print(F("Connected event, central: "));
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  //DebugLEDOnOff(); 
  Serial.print(F("Disconnected event, central: "));
  Serial.println(central.address());
}

//Writeイベント
void switchCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  //DEbug
  //DebugLEDOnOff(); 
  
  // central wrote new value to characteristic, update LED
  Serial.print(F("Characteristic event, writen: "));
  
  //Write受信データサイズ
  LED_RecieveLength = characteristic.valueLength();

  //Writeデータ最大サイズ(16)回ループ
  //LED_RecieveValue[16] = {};
  memset(LED_RecieveValue, '¥0', sizeof(LED_RecieveValue));
  for(int loopcount = 0; loopcount < 16; loopcount++){
    LED_RecieveValue[loopcount] = *(characteristic.value()+loopcount);    
  }

  //ヘッダとペイロード切り分け、ヘッダは必要部分だけ抽出。ペイロードは配列に格納。
  if (LED_RecieveValue[0] == 0x77) {
    isWritting          = true;
    ConcateCount        = 0;
    PayloadLineCounter  = 0;
    save_Flash          = LED_RecieveValue[6];
    save_Marqee         = LED_RecieveValue[7];
    save_speed_mode     = LED_RecieveValue[8];
    //DebugLEDOnOff();
    //ヘッダ抽出 ※後で配列でなく変数にする。 
      
  } else {
    //Debug
    //DebugLEDOnOff();
    
    //ヘッダ抽出続き
    if(ConcateCount == 1){
      //ヘッダから文字数を取得
      B1122_CharLength = LED_RecieveValue[1];
      //ペーロードのサイズを算出 ( 文字数 * 11 )
      WritePayloadSize   = B1122_CharLength * ONE_CHAR_SIZE;
      
      //ペイロードの数 16byteのペイロードの数を算出
      //文字データ数(文字数x11Byte) / 16 + 1(あまりが出た時)
      B1122_PayloadLines = WritePayloadSize / MAX_WRITE_LENGTH;
      SurplusLength = WritePayloadSize % MAX_WRITE_LENGTH;
      if( SurplusLength != 0){
        B1122_PayloadLines = B1122_PayloadLines + 1;
      }

      //配列初期化(文字数 x 11)
      //LED_payload[WritePayloadSize] = {};
      memset(LED_payload, '¥0', sizeof(LED_payload));
    //ペイロード格納
    }else if(ConcateCount >= 4){
      
      //最終行のあまりがでるまでPayloadをマージ格納
      for(margecount = 0; margecount < MAX_WRITE_LENGTH; margecount++){
        //ペイロードカウントがMAXペイロード数未満
        if((PayloadLineCounter < B1122_PayloadLines)){
          LED_payload[(PayloadLineCounter * MAX_WRITE_LENGTH) + margecount] = LED_RecieveValue[margecount];
        }else{
          //最後のPayload
          //ループカウンタがデータが埋まってるところまでデータ移行
          if(margecount < (MAX_WRITE_LENGTH - SurplusLength)){
            LED_payload[(PayloadLineCounter * MAX_WRITE_LENGTH) + margecount] = LED_RecieveValue[margecount];
          }       
        }
      }
      PayloadLineCounter++;
    }
    //
    /**/
    //連結カウンタが最大値(全データ受信)
    if(PayloadLineCounter == B1122_PayloadLines){  
      int charcount        = 0;  //文字byteデータ格納ループカウンタ
      int B1144dotcout     = 0;  //1文字(11byte)カウンタ
      int MAX7219dotcout   = 0;  //1文字(8byte)カウンタ
      //文字列(文字数x8)分の配列初期化
      //MAX7219__payload[B1122_CharLength * 8] = {};
      memset(MAX7219__payload, '¥0', sizeof(MAX7219__payload));
      //文字数分ループ
      //
      //Debug ダミーデータ(NORA)を格納。
      /**
      MAX7219__payload[5*8] = {};
      LED_payload[55] = {};
      uint8_t deb[55] = {0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 
                         0x00, 0x00, 0xfe, 0xc0, 0xc0, 0xfe, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 
                         0x00, 0x00, 0xfe, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 
                         0x00, 0x00, 0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00,
                         0x00, 0x00, 0x7C, 0x00, 0xFE, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00};
      for(int t=0;t<55;t++){
        LED_payload[t] = deb[t];
      }
      B1122_CharLength = 5;
      /**/
      //Debug
      //

      int display_mode = save_speed_mode & 0x0F;
      if(display_mode == MODE_RIGHT){
        for(int blankcount = 0; blankcount < blanklength; blankcount++){
          MAX7219__payload[ MAX7219dotcout] = 0x00;
          MAX7219dotcout++;
        }        
      }
      for(charcount = 0; charcount < B1122_CharLength; charcount++){
        //11byte分ループ
        for(B1144dotcout = 0; B1144dotcout < ONE_CHAR_SIZE; B1144dotcout++){
          //上2列、下1列は格納しない。
          if((B1144dotcout != 0) && (B1144dotcout != 1) && (B1144dotcout != 10) ){
            MAX7219__payload[MAX7219dotcout] = LED_payload[(charcount*11) + B1144dotcout];
            MAX7219dotcout++;
          }
        }
      }
      if(display_mode != MODE_RIGHT){
        for(int blankcount = 0; blankcount < blanklength; blankcount++){
          MAX7219__payload[ MAX7219dotcout ] = 0x00;
          MAX7219dotcout++;
        }
      }
      //文字列長差し替え
      MAX7219__CharLength = B1122_CharLength + 4;
      display_Flash       = save_Flash;
      display_Marqee      = save_Marqee;
      display_speed_mode  = save_speed_mode;
      //書き込み中フラグをオフ
      isWritting = false;
    }
  }
  ConcateCount++;
  //delay(50);
}


void MAX7219Display(uint8_t *dotArray,int charlen, uint8_t Flash, uint8_t Marquee, uint8_t SpMode){
  uint8_t  font[8];
  uint8_t* ptr = MAX7219_getBuffer();
  uint8_t  Mode = 0;
  uint8_t  Speed = 0;
  uint8_t* WkdotArray;
  uint8_t  wkbuff[8*4];

  memset(wkbuff, '¥0', sizeof(wkbuff));

  // モードの取得
  Mode = SpMode & 0x0F;
  // スピードの取得
  Speed = SpMode >> 4;
  Speed = Speed & 0x0F;
  
  /* LEDの表示を初期化する */
  // MAX7219_clear();
  
  // 4文字以下なら4文字にする
  if(charlen < 4){
    memcpy(wkbuff, dotArray, charlen*8);
    charlen = 4;
    WkdotArray = wkbuff;
  }
  else{
    WkdotArray = dotArray;    
  }
 
  /* スクロール表示 */
  // 右スクロール
  if(Mode == MODE_RIGHT){
    for (int8_t x=-8*charlen-16; x<=17; x++) {
      uint16_t n = 0;
      matrix.clear();
      matrix1.clear();
      matrix.setCursor(x,0);
      matrix1.setCursor(x+16,0);
      for(int Mcount = 0; Mcount < charlen; Mcount++){
        memcpy(font, WkdotArray+(Mcount*8), 8);
        matrix.drawBitmap(x+n,0,font,8,8,1);
        matrix1.drawBitmap(x+16+n,0,font,8,8,1);
        n+=8;
      }
      matrix.writeDisplay();
      matrix1.writeDisplay();
      delay(100+((3-Speed)*20));
    }
  }
  // 左スクロール
  else{
    for (int8_t x=17; x>=-8*charlen-16; x--) {
      uint16_t n = 0;
      matrix.clear();
      matrix1.clear();
      matrix.setCursor(x,0);
      matrix1.setCursor(x+16,0);
      for(int Mcount = 0; Mcount < charlen; Mcount++){
        memcpy(font, WkdotArray+(Mcount*8), 8);
        matrix.drawBitmap(x+n,0,font,8,8,1);
        matrix1.drawBitmap(x+16+n,0,font,8,8,1);
        n+=8;
      }
      matrix.writeDisplay();
      matrix1.writeDisplay();
      delay(100+((3-Speed)*20));
    }
  }

  return;
}


void DebugLEDOnOff(){
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);   
  delay(100);
}


// for example
void TestLEDMatrix(){
  uint8_t buf[8];
  char *str="日本語のテストです";
  for (int8_t x=17; x>=-88; x--) {
    char *ptr = str;
    uint16_t n = 0;
    matrix.clear();
    matrix1.clear();
    matrix.setCursor(x,0);
    matrix1.setCursor(x+16,0);
    while(*ptr){
      ptr = getFontData(buf,ptr,true);
      if(!ptr)
        break;
      matrix.drawBitmap(x+n,0,buf,8,8,1);
      matrix1.drawBitmap(x+16+n,0,buf,8,8,1);
      n+=8;
    }
    matrix.writeDisplay();
    matrix1.writeDisplay();
    delay(20);
  }
}
