// LoRaWAN template sketch
bool DBG = true; // debug switch

// macros for debug calls to Serial.printf, with (D) and without (DD)
// new line, and to Serial.println (DDD)
#define D(args...)   { if(DBG) { Serial.printf(args); Serial.println(); } }
#define DD(args...)  { if(DBG) Serial.printf(args); }
#define DDD(s)       { if(DBG) Serial.println(s); }

#include <lmic.h>               // IBM LMIC (LoraMAC-in-C) library
#include <hal/hal.h>            // hardware abstraction for LMIC on Arduino
#include <SPI.h>                // the SPI bus
#include <Adafruit_GFX.h>       // core graphics library
#include "Adafruit_HX8357.h"    // tft display local hacked version
#include "Adafruit_STMPE610.h"  // touch screen local hacked version
#include <Wire.h>               // I²C comms on the Arduino
#include "IOExpander.h"         // unPhone's IOExpander (controlled via I²C)

// the LCD and touch screen
#define TFT_DC   33
Adafruit_HX8357 tft =
  Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);
Adafruit_STMPE610 ts = Adafruit_STMPE610(IOExpander::TOUCH_CS);
// calibration data for converting raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

// battery management
byte BM_I2Cadd = 0x6b;
byte BM_Watchdog = 0x05; // Charge Termination/Timer Control Register
byte BM_OpCon    = 0x07; // Misc Operation Control Register
byte BM_Status   = 0x08; // System Status Register 
byte BM_Version  = 0x0a; // Vender / Part / Revision Status Register 

//LoRaWAN configuration:
// LoRaWAN NwkSKey, network session key
// This is a dummy key, which should be replaced by your individual key
// Copy from ttn device overview webpage - Network Session Key - msb format
static const PROGMEM u1_t NWKSKEY[16] = { 0x6A, 0x65, 0x49, 0x77, 0xCC, 0x7F, 0xC3, 0xD8, 0x8C, 0x33, 0xD4, 0xDE, 0x82, 0x8A, 0x77, 0xE8 };

// LoRaWAN AppSKey, application session key
// This is a dummy key, which should be replaced by your individual key
// Copy from ttn device overview webpage - App Session Key - msb format
static const u1_t PROGMEM APPSKEY[16] = { 0x8C, 0x77, 0xBC, 0xCE, 0xE4, 0xB9, 0x84, 0x48, 0xC4, 0x8D, 0x01, 0xD6, 0x13, 0xAD, 0x6C, 0x9F };

// LoRaWAN end-device address (DevAddr)
// This is a dummy key, which should be replaced by your individual key
// Copy from ttn device overview webpage - Device Address - msb format
static const u4_t DEVADDR = 0x260112F9 ;

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

uint8_t mydata[] = "Hello, World!";
uint8_t loopcount[] = " Loops: "; 
static osjob_t sendjob;

// schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// pin mapping
const lmic_pinmap lmic_pins = {  // modified to suit connection on unphone
  .nss = IOExpander::LORA_CS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = IOExpander::LORA_RESET,
  .dio = {39, 34, LMIC_UNUSED_PIN}  
};

void onEvent (ev_t ev) {
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            DDD(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            DDD(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            DDD(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            DDD(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            DDD(F("EV_JOINING"));
            break;
        case EV_JOINED:
            DDD(F("EV_JOINED"));
            break;
        case EV_RFU1:
            DDD(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            DDD(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            DDD(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            DDD(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              DDD(F("Received ack"));
            if (LMIC.dataLen) {
              DDD(F("Received "));
              DDD(LMIC.dataLen);
              DDD(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            DDD(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            DDD(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            DDD(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            DDD(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            DDD(F("EV_LINK_ALIVE"));
            break;
         default:
            DDD(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j) {
    // check if there is not a current TX/RX job running
    if(LMIC.opmode & OP_TXRXPEND) {
        D("OP_TXRXPEND, not sending");
    } else {
        // prepare upstream data transmission at the next possible time
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        loopcount[0]=+1;
        LMIC_setTxData2(1, loopcount, sizeof(loopcount)-1, 0);        
        D("Packet queued");
    }
    // next TX is scheduled after TX_COMPLETE event
}

void setup() {
  if (DBG==true) Serial.begin(115200);
  Wire.setClock(100000); // TODO higher rates trigger an IOExpander bug
  Wire.begin();
  IOExpander::begin();
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);

  bool powerGood = // bit 2 of status register indicates if USB connected
    bitRead(getRegister(BM_I2Cadd, BM_Status), 2);

  if(!inputPwrSw) {  // when power switch off
    if(!powerGood) { // and usb unplugged we go into shipping mode
      setShipping(true);
    } else { // power switch off and usb plugged in we sleep
      esp_sleep_enable_timer_wakeup(1000000); // sleep time is in uSec
      esp_deep_sleep_start();
    }
  }
 
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
  ts.begin();
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_WHITE);
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);

  // LMIC init
  DDD("doing LMIC os_init()...");
  os_init(); // 
  
  // reset the MAC state; session and pending data transfers will be discarded
  DDD("doing LMIC_reset()...");
  LMIC_reset();

  // Start job
  do_send(&sendjob);
}

void loop(void) {
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);

  bool powerGood = // bit 2 of status register indicates if USB connected
    bitRead(getRegister(BM_I2Cadd, BM_Status), 2);

  if(!inputPwrSw) {  // when power switch off...
    if(!powerGood) { // and usb unplugged we go into shipping mode
      setShipping(true);
    } else { // power switch is off and usb plugged in: we sleep
      esp_sleep_enable_timer_wakeup(1000000); // sleep time is in uSec
      esp_deep_sleep_start();
    }
  }
  os_runloop_once(); 
  delay(50);
}

void setShipping(bool value) {
  byte result;
  if(value) {
    result=getRegister(BM_I2Cadd, BM_Watchdog);  // state of timing register
    bitClear(result, 5);                         // clear bit 5...
    bitClear(result, 4);                         // and bit 4 to disable...
    setRegister(BM_I2Cadd, BM_Watchdog, result); // WDT (REG05[5:4] = 00)

    result=getRegister(BM_I2Cadd, BM_OpCon);     // operational register
    bitSet(result, 5);                           // set bit 5 to disable...
    setRegister(BM_I2Cadd, BM_OpCon, result);    // BATFET (REG07[5] = 1)
  } else {
    result=getRegister(BM_I2Cadd, BM_Watchdog);  // state of timing register
    bitClear(result, 5);                         // clear bit 5...
    bitSet(result, 4);                           // and set bit 4 to enable...
    setRegister(BM_I2Cadd, BM_Watchdog, result); // WDT (REG05[5:4] = 01)

    result=getRegister(BM_I2Cadd, BM_OpCon);     // operational register
    bitClear(result, 5);                         // clear bit 5 to enable...
    setRegister(BM_I2Cadd, BM_OpCon, result);    // BATFET (REG07[5] = 0)
  }
}

void setRegister(byte address, byte reg, byte value) {
  write8(address, reg, value);
}

byte getRegister(byte address, byte reg) {
  byte result;
  result=read8(address, reg);
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
