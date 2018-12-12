#include <SPI.h>                // the SPI bus
#include <Adafruit_GFX.h>       // core graphics library
#include "Adafruit_HX8357.h"    // tft display local hacked version
#include <Wire.h>               // I²C comms on the Arduino
#include "IOExpander.h"         // unPhone's IOExpander (controlled via I²C)
#include <Adafruit_Sensor.h>    // base class etc. for sensor abstraction
#include <Adafruit_LSM303_U.h>  // the accelerometer sensor

// power management chip config
byte I2Cadd = 0x6b;      // I2C address of the PMU
byte BM_Watchdog = 0x05; // charge termination/timer control register
byte BM_OpCon    = 0x07; // misc operation control register
byte BM_Status   = 0x08; // system status register 
byte BM_Version  = 0x0a; // vender / part / revision status register 

// the LCD
#define TFT_DC   33      // this isn't on the IO expander
Adafruit_HX8357 tft =
  Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);

// the accelerometer (which we're using for attitude sensing)
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// x and y coords of the etching pen
int penx = 20;
int peny = 0;
int oldx = 0;
int oldy = 0;

// setup ////////////////////////////////////////////////////////////////////
void setup(void) {
  Serial.begin(115200);
  Serial.println("Starting");
  Serial.println("Version 1.01");
  Serial.println("etch-a-sketch by gareth");

  /* Initialise the sensor */
  Wire.setClock(100000);
  if(!accel.begin()) {
    /* There was a problem detecting the sensor ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  
  IOExpander::begin();
  checkPowerSwitch(); // check if the power switch is now off & if so shutdown
    
  tft.begin(HX8357D); // start the LCD driver chip
  IOExpander::digitalWrite(IOExpander::BACKLIGHT,HIGH); // backlight on

  tft.fillScreen(HX8357_BLACK);
  tft.drawLine(20,459,319,459,HX8357_WHITE);
  tft.setCursor(5,5);
  tft.println("air");
  
  tft.drawLine(20,459,20,20,HX8357_WHITE);
  tft.setCursor(279,470);
  tft.println("time");

  
}

void loop() {
  checkPowerSwitch();
  int rand = random(20,309);
  barGraph(rand);
  delay(500);
}

//analysis
void lineGraph(int current){
  Serial.println(current);
  if(penx<319 && penx > 20){
    oldx = penx;
    oldy = peny;
    penx += 25;
    peny = current;
  }
  else{
    penx = 20;
    oldx = 20;
    oldy = peny;
    penx +=25;
    peny = current;
  
    tft.fillScreen(HX8357_BLACK);
    tft.drawLine(20,459,319,459,HX8357_WHITE);
    tft.setCursor(5,5);
    tft.println("air");
    
    tft.drawLine(20,459,20,20,HX8357_WHITE);
    tft.setCursor(279,470);
    tft.println("time");

    tft.drawLine(20,240,319,240,HX8357_RED);
    tft.setCursor(259,240);
    tft.println("optimum");
  }
  tft.drawLine(oldx,oldy,penx, peny, HX8357_WHITE);
}

void barGraph(int current){
  Serial.println(current);
  if(penx< 319 && penx > 20){
    penx +=25;
    peny = current;
  }
  else{
    penx = 20;
    penx += 25;
    peny = current;
    tft.fillScreen(HX8357_BLACK);
    tft.drawLine(20,459,319,459,HX8357_WHITE);
    tft.setCursor(5,5);
    tft.println("air");
    
    tft.drawLine(20,459,20,20,HX8357_WHITE);
    tft.setCursor(279,470);
    tft.println("time");

  
    tft.drawLine(20,240,319,240,HX8357_RED);
    tft.setCursor(259,240);
    tft.println("optimum");
  }
  tft.fillRect(penx, peny, 25, (459-peny) , HX8357_GREEN);
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
