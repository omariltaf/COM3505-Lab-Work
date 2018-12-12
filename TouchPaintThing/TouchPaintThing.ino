// TouchPaintThing
// a demo of the unPhone's touch screen and TFT LCD
// derived from Adafruit's example for http://www.adafruit.com/products/2050
// by Hamish & Gareth

#include <SPI.h>                // the SPI bus
#include <Adafruit_GFX.h>       // core graphics library
#include "Adafruit_HX8357.h"    // tft display local hacked version
#include "Adafruit_STMPE610.h"  // touch screen local hacked version
#include <Wire.h>               // I²C comms on the Arduino
#include "IOExpander.h"         // unPhone's IOExpander (controlled via I²C)

// power management chip config
byte I2Cadd = 0x6b;      // I2C address of the PMU
byte BM_Watchdog = 0x05; // charge termination/timer control register
byte BM_OpCon    = 0x07; // misc operation control register
byte BM_Status   = 0x08; // system status register 
byte BM_Version  = 0x0a; // vender / part / revision status register 

// macros for debug calls to Serial.printf, with (D) and without (DD)
// new line, and to Serial.println (DDD)
bool DBG = false; // debug switch
#define D(args...)   { if(DBG) { Serial.printf(args); Serial.println(); } }
#define DD(args...)  { if(DBG) Serial.printf(args); }
#define DDD(s)       { if(DBG) Serial.println(s); }

// parent for device element classes
class Chunk {
public:
  int begin();  // initialisation
  void test();  // validation
};

// esp32 feather pin definitions
#define TFT_DC   33
Adafruit_HX8357 tft =
  Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);
Adafruit_STMPE610 ts =
  Adafruit_STMPE610(IOExpander::TOUCH_CS);

// calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

// size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
// #define PENRADIUS 3
#define PENRADIUS 9
// #define PENRADIUS 15

// a menu along one of the short sides of the display
class Menu: public Chunk {
public:
  // colour definitions
  const int BLACK =     HX8357_BLACK;
  const int BLUE =      HX8357_BLUE;
  const int RED =       HX8357_RED;
  const int GREEN =     HX8357_GREEN;
  const int CYAN =      HX8357_CYAN;
  const int MAGENTA =   HX8357_MAGENTA;
  const int YELLOW =    HX8357_YELLOW;
  const int WHITE =     HX8357_WHITE;

  // menu boxes
  static const int NUMBOXES = 8;
  const int colours2Boxes[NUMBOXES] = {
    RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE, BLACK,
  };

  int oldcolor, currentcolor;

  void init() {
    for(int i = 0; i < NUMBOXES; i++) // fill the color selection boxes
      tft.fillRect(i * BOXSIZE, 0, BOXSIZE, BOXSIZE, colours2Boxes[i]);
    tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE); // select red box
    currentcolor = RED;
  }
  void select(TS_Point p) { // make selection for point inside menu area (Y)
    oldcolor = currentcolor;
    bool changedMenu = false;
 
    int boxXOrigin;
    for(int i = 0; i < NUMBOXES; i++) { // put white border around selection
      boxXOrigin = i * BOXSIZE;
      if(p.x < ((i + 1) * BOXSIZE) ) {
        currentcolor = colours2Boxes[i];
        if(i == 6) // white box: red border
          tft.drawRect(boxXOrigin, 0, BOXSIZE, BOXSIZE, RED);
        else {
          tft.drawRect(boxXOrigin, 0, BOXSIZE, BOXSIZE, WHITE);
          if(i == 7) changedMenu = true; // black box, menu switcher
        }
 
        break;
      }
    }
 
    if(oldcolor != currentcolor) { // refill previous selection
      for(int i = 0; i < NUMBOXES; i++)
        if(colours2Boxes[i] == oldcolor)
          tft.fillRect(i * BOXSIZE, 0, BOXSIZE, BOXSIZE, oldcolor);
 
      if(changedMenu) Serial.println("change menu...");
      Serial.print("changed colour to "); 
      Serial.println(currentcolor);
    }
  }
  void test() {
    D("Menu::test()\n");
  }
};
Menu m;

// setup ////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.println("TouchPaintThing");
  Wire.setClock(100000);
  Wire.begin();
  IOExpander::begin();
  checkPowerSwitch(); // check if the power switch is now off & if so shutdown
  delay(100);

  if(! ts.begin())
    Serial.println("failed to start touchscreen controller");
  else
    Serial.println("touchscreen started");
  tft.begin(HX8357D);
  IOExpander::digitalWrite(IOExpander::BACKLIGHT,HIGH);
  tft.fillScreen(m.BLACK);
  m.init();
}

// loop /////////////////////////////////////////////////////////////////////
int loopiter = 0;
void loop(void) {
  checkPowerSwitch(); // check if the power switch is now off & if so shutdown

  // retrieve a point  
  TS_Point p = ts.getPoint();
  // Serial.print("X = "); Serial.print(p.x); Serial.print("\tY = ");
  // Serial.print(p.y);  Serial.print("\tPressure = "); Serial.println(p.z);

  /* to debug the point logic:
  DBG = true;
  if(loopiter++ % 1000 == 0) {
    DD("p(%d/%d/%d); ", p.x, p.y, p.z)
    m.test();
  }
  */
   
  // scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
  p.y = map(p.y, TS_MAXY, TS_MINY, 0, tft.height());
  Serial.println(p.x);
  Serial.println(p.y);

  // respond to the touch
  if(p.y < BOXSIZE) // we're in the menu area
    m.select(p);
  if(((p.y-PENRADIUS) > 0) && ((p.y+PENRADIUS) < tft.height())) // draw
    tft.fillCircle(p.x, p.y, PENRADIUS, m.currentcolor);
}

// power management chip API /////////////////////////////////////////////////
void checkPowerSwitch() {
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);

  // bit 2 of status register indicates if USB connected
  bool powerGood = bitRead(getRegister(BM_Status),2);

  if(!inputPwrSw) {  // when power switch off
    if(!powerGood) { // and usb unplugged we go into shipping mode
      Serial.println("Setting shipping mode to true");
      Serial.print("Status is: ");
      Serial.println(getRegister(BM_Status));
      delay(500); // allow serial buffer to empty
      setShipping(true);
    } else { // power switch off and usb plugged in we sleep
      Serial.println("sleeping now");
      Serial.print("Status is: ");
      Serial.println(getRegister(BM_Status));
      delay(500); // allow serial buffer to empty
      esp_sleep_enable_timer_wakeup(1000000); // sleep time is in uSec
      esp_deep_sleep_start();
    }
  }
}
void setShipping(bool value) {
  byte result;
  if(value) {
    result=getRegister(BM_Watchdog); // read current state of timing register
    bitClear(result, 5);             // clear bit 5
    bitClear(result, 4);             // and bit 4
    setRegister(BM_Watchdog,result); // disable watchdog tmr (REG05[5:4] = 00)

    result=getRegister(BM_OpCon);    // current state of operational register
    bitSet(result, 5);               // set bit 5
    setRegister(BM_OpCon,result);    // to disable BATFET (REG07[5] = 1)
  } else {
    result=getRegister(BM_Watchdog); // Read current state of timing register
    bitClear(result, 5);             // clear bit 5
    bitSet(result, 4);               // and set bit 4
    setRegister(BM_Watchdog,result); // enable watchdog tmr (REG05[5:4] = 01)

    result=getRegister(BM_OpCon);    // current state of operational register
    bitClear(result, 5);             // clear bit 5
    setRegister(BM_OpCon,result);    // to enable BATFET (REG07[5] = 0)
  }
}

// I2C helpers to drive the power management chip
void setRegister(byte reg, byte value) {
  write8(I2Cadd,reg, value);
}
byte getRegister(byte reg) {
  byte result;
  result=read8(I2Cadd,reg);
  return result;
}
void write8(byte address, byte reg, byte value) {
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}
byte read8(byte address, byte reg) {
  byte value;
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}
