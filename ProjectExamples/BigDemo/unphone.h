// unphone.h
// core definitions and includes

#ifndef UNPHONE_H
#define UNPHONE_H

#include <SD.h>                 // the SD card
#include <lmic.h>               // IBM LMIC (LoraMAC-in-C) library
#include <hal/hal.h>            // hardware abstraction for LMIC on Arduino
#include <SPI.h>                // the SPI bus
#include <Adafruit_GFX.h>       // core graphics library
#include <Adafruit_HX8357.h>    // tft display
#include <Adafruit_STMPE610.h>  // touch screen
#include <Wire.h>               // I²C comms on the Arduino
#include <IOExpander.h>         // unPhone's IOExpander (controlled via I²C)
#include <Adafruit_Sensor.h>    // base class etc. for sensor abstraction
#include <Adafruit_LSM303_U.h>  // the accelerometer sensor
#include <driver/i2s.h>         // ESP I²S bus
#include <Adafruit_VS1053.h>    // the audio chip

#ifdef UNPHONE_RCSWITCH
#include <RCSwitch.h>
#endif

// out-of-repo key data & etc.
#include "grunge.h"

// hardware version (set in Makefile):
// 0: one of the breadboard variants
// 1: the bare feather/featherwing TS/TFT (3.5")
// 2: first Pimo proto
#ifndef UNPHONE_VERSION
# define UNPHONE_VERSION 0
#endif

#ifndef BUILD_VERSION
# define BUILD_VERSION "none"
#endif

#ifndef DEV_BUILD
# define DEV_BUILD 0
#endif

// macros for debug (and error) calls to Serial.printf, with (D) and without
// (DD) new line, and to Serial.println (DDD)
#ifdef UNPHONE_PRODUCTION_BUILD
# define D(args...)
# define E(args...)
#else
extern bool UNPHONE_DBG;     // debug switch
# define D_ON  UNPHONE_DBG = 1;
# define DD(args...)  if(UNPHONE_DBG) Serial.printf(args);
# define D_OFF UNPHONE_DBG = 0;
# define D(args...) if(UNPHONE_DBG) printf(args);
# define E(args...) printf(args);
#endif
/* TODO
void logToWeb(
#define L(args...) { printf(args); if(LOG_TO_WEB) { logToWeb(args); } }
*/

// delay/yield/timing and time-slicing macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis
extern int loopIter; // main task loop iteration counter

char *getMAC(char *buf);

// the UI
enum ui_modes_t { // interface modes
  ui_touchpaint = 1, ui_configure, ui_text, ui_testcard, ui_mic, ui_music, ui_qwerty
};

// power management chip API /////////////////////////////////////////////////
extern byte BM_I2Cadd;          // the chip lives here on I²C
extern byte BM_Watchdog;        // charge termination/timer control register
extern byte BM_OpCon;           // misc operation control register
extern byte BM_Status;          // system status register 
extern byte BM_Version;         // vender / part / revision status register 
bool checkPowerSwitch();        // returns status of USB power (true = on)
void setShipping(bool value);   // tells power management chip to shut down
void setRegister(byte address, byte reg, byte value);   //
byte getRegister(byte address, byte reg);               // I²C...
void write8(byte address, byte reg, byte value);        // ...helpers
byte read8(byte address, byte reg);                     // 

// the accelerometer /////////////////////////////////////////////////////////
extern Adafruit_LSM303_Accel_Unified accel;

// the LCD and touch screen //////////////////////////////////////////////////
#define TFT_DC    33
extern Adafruit_HX8357 tft;
extern Adafruit_STMPE610 ts;
// calibration data for converting raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX  100
#define TS_MINY  100
#define TS_MAXY 3750
class TestScreen { // screens for the test routine
public:
  static void init();
  static void activate(bool);
  static bool on();
  static void fail(char *);
  static bool inSquares(TS_Point);
  static void testSequence(bool);
};
extern bool testScreenActive;

// the VS1053 music and synth chip ///////////////////////////////////////////
#define VS1053_DREQ 36
extern Adafruit_VS1053_FilePlayer musicPlayer;

// the mic, and the I²S bus /////////////////////////////////////////////////
#define MIC_DOUT 35
#define MIC_BCLK 13
#define MIC_LRCL  4
void i2s_config();
uint32_t read_i2s();

// the IR LED pins //////////////////////////////////////////////////////////
#define IR_LEDS  12

// the LoRa board and TTN LoRaWAN ///////////////////////////////////////////
extern osjob_t sendjob;
void os_getArtEui (u1_t*);
void os_getDevEui (u1_t*);
void os_getDevKey (u1_t*);
void lmic_init();
void lmic_do_send(osjob_t*);

#endif
