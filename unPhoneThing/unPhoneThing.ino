// unPhoneThing.ino
// demonstrates main characteristics and usage of the unPhone

/////////////////////////////////////////////////////////////////////////////
// ******* CAUTION, HERE BE DRAGONS! *******
// this is a rough-and-ready test rig for the unPhone, NOT a programming
// tutorial or an API library! copy and use at your peril!
/////////////////////////////////////////////////////////////////////////////

#include <lmic.h>               // IBM LMIC (LoraMAC-in-C) library
#include <hal/hal.h>            // hardware abstraction for LMIC on Arduino
#include <SPI.h>                // the SPI bus
#include <Adafruit_GFX.h>       // core graphics library
#include "Adafruit_HX8357.h"    // tft display local hacked version
#include "Adafruit_STMPE610.h"  // touch screen local hacked version
#include <Wire.h>               // I²C comms on the Arduino
#include "IOExpander.h"         // unPhone's IOExpander (controlled via I²C)
#include <Adafruit_Sensor.h>    // base class etc. for sensor abstraction
#include <Adafruit_LSM303_U.h>  // the accelerometer sensor
#include "driver/i2s.h"         // ESP I²S bus
#include "SD.h"                 // the SD card
#include "Adafruit_VS1053.h"    // the audio chip

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

// the accelerometer (with a unique ID)
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// the LCD and touch screen
#define TFT_DC   33
Adafruit_HX8357 tft =  Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);
Adafruit_STMPE610 ts = Adafruit_STMPE610(IOExpander::TOUCH_CS);
// calibration data for converting raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

// a test set of boxes
class Menu: public Chunk {
public:
  void init() {
    tft.setTextSize(3);
    tft.setCursor(65, 110);
    tft.setTextColor(HX8357_BLACK);
    tft.print("Screen Test");
    tft.setCursor(120, 170);
    tft.setTextColor(HX8357_RED);
    tft.print("Red");  
    tft.setCursor(120, 200);
    tft.setTextColor(HX8357_GREEN);
    tft.print("Green");
    tft.setCursor(120, 230);
    tft.setTextColor(HX8357_BLUE);
    tft.print("Blue");
    tft.fillRect(110, 320, 100, 40 , HX8357_BLACK);
    tft.setCursor(120, 330);
    tft.setTextColor(HX8357_WHITE);
    tft.print("NEXT!");
    tft.fillRect(30, 30, 70, 70 , HX8357_GREEN);
    tft.fillRect(220, 30, 70, 70 , HX8357_RED);
    tft.fillRect(30, 380, 70, 70 , HX8357_BLUE);
    tft.fillRect(220, 380, 70, 70 , HX8357_CYAN);
  }
  
  void fail() {
    tft.fillScreen(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(65, 110);
    tft.setTextColor(HX8357_BLUE);
    tft.print("!!  FAIL  !!");
    tft.setCursor(65, 250);
  }
};
Menu m;

// battery management
byte BM_I2Cadd = 0x6b;
byte BM_Watchdog = 0x05; // Charge Termination/Timer Control Register
byte BM_OpCon    = 0x07; // Misc Operation Control Register
byte BM_Status   = 0x08; // System Status Register 
byte BM_Version  = 0x0a; // Vender / Part / Revision Status Register 

void i2s_config() {
  // http://esp-idf.readthedocs.io/en/latest/api/peripherals/i2s.html
  // input
  i2s_config_t i2s_in_config = {
    mode: (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    sample_rate: 44100,
    bits_per_sample: (i2s_bits_per_sample_t)32,
    channel_format: I2S_CHANNEL_FMT_RIGHT_LEFT,
    communication_format: (i2s_comm_format_t)
      (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    intr_alloc_flags: ESP_INTR_FLAG_LEVEL1,
    dma_buf_count: 14,
    dma_buf_len: 64
  };
  i2s_pin_config_t i2s_in_pin_config = {
    bck_io_num: 13,
    ws_io_num: 4,
    data_out_num: -1, //Not used
    data_in_num: 35
  };

   pinMode(35, INPUT);  // aka pin TX connected to Data OUT (DOUT)
   pinMode(13, OUTPUT); // aka RX to Bit Clock (BCLK) aka Clock
   pinMode(4, OUTPUT);//aka A5 to Left Right Clock (LRCL) aka Word Select (WS)

  i2s_driver_install((i2s_port_t)0, &i2s_in_config, 0, NULL);
  i2s_set_pin((i2s_port_t)0, &i2s_in_pin_config);
}

uint32_t read_i2s() {
  uint32_t sample_val[2] = {0, 0};
  uint8_t bytes_read = i2s_pop_sample(
    (i2s_port_t) 0, (char *) sample_val, portMAX_DELAY
  );
  return sample_val[0] << 5;
}

#define VS1053_DREQ 36
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(
  IOExpander::MUSIC_RESET, IOExpander::MUSIC_CS, IOExpander::MUSIC_DCS,
  VS1053_DREQ, IOExpander::SD_CS
);


// LoRaWAN NwkSKey, network session key
// this is the default Semtech key, which is used by the prototype TTN
// network initially
static const PROGMEM u1_t NWKSKEY[16] = {
  0xC7, 0xFC, 0xF2, 0xF1, 0xAC, 0x8D, 0x07, 0x09, 0x4F, 0x33, 0xD4,
  0xAA, 0xDE, 0x67, 0x17, 0xA9
};

// LoRaWAN AppSKey, application session key
// this is the default Semtech key, which is used by the prototype TTN
// network initially
static const u1_t PROGMEM APPSKEY[16] = {
  0xBE, 0xEF, 0xA3, 0xB7, 0x59, 0xAA, 0x79, 0x49, 0xC6, 0x06, 0x11,
  0x90, 0xAE, 0xEB, 0x47, 0x8A
};

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
static const u4_t DEVADDR = 0x260115CC ; // <-- change address for every node!

// these callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain)
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

uint8_t mydata[] = "Hello, World!";
int loops=1;
uint8_t loopcount[] = " Loops: "; 
static osjob_t sendjob;

// schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// pin mapping
const lmic_pinmap lmic_pins = {
  .nss = IOExpander::LORA_CS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = IOExpander::LORA_RESET,
  .dio = {39, 34, LMIC_UNUSED_PIN}  // modified to suit connection on unphone
};

void onEvent(ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if(LMIC.dataLen) {
                // data received in rx slot after tx
                Serial.print(F("Data Received: "));
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
            }
            // schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
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

int stage=1;

void setup() {
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

  int version = IOExpander::getVersionNumber(), spin;
  if(version == 7) spin = 4;

  Serial.begin(115200);
  Serial.printf("starting, running on spin %d\n", spin);
  
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_WHITE);
  m.init();
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);

  if(! ts.begin()) {
    D("failed to start touchscreen controller");
    m.fail();
    tft.println("TOUCH");
    delay(3000);
  } else {
    D("touchscreen started");
  }
  
  if(!accel.begin())
  {
    D("Failed to start accelerometer");
    m.fail();
    tft.println("ACCEL");
    delay(3000);
  }
  
  i2s_config();

  pinMode(12, OUTPUT);     // IR_LED pin
  
  IOExpander::digitalWrite(IOExpander::SD_CS, LOW);
  if(!SD.begin(-1)) {
    D("Card Mount Failed");
    m.fail();
    tft.println("SD CARD");
    delay(3000);
  }
  IOExpander::digitalWrite(IOExpander::SD_CS, HIGH);

  if(! musicPlayer.begin()) { // initialise the music player
    D("Couldn't find VS1053, do you have the right pins defined?");
    m.fail();
    tft.println("AUDIO");
    delay(3000);
  } else {
    D("VS1053 found");
  }
   
  // LMIC init
  Serial.println("doing os_init()...");
  tft.println("LoRa Sending");
  os_init(); // if lora fails then will stop, allowing panic mess to be seen
  tft.fillScreen(HX8357_WHITE);
  m.init();
  tft.setCursor(65, 270);
  tft.setTextColor(HX8357_MAGENTA);
  tft.println("LoRa Sent");

  // reset the MAC state; session and pending data transfers will be discarded
  Serial.println("doing LMIC_reset()...");
  LMIC_reset();

  // set static session parameters instead of dynamically establishing session
  // by joining the network, precomputed session parameters are provided
  #ifdef PROGMEM
  // on AVR, these values are stored in flash and only copied to RAM
  // once; copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  Serial.println("doing LMIC_setSession() PROGMEM...");
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
  #else
  // if not running an AVR with PROGMEM, just use the arrays directly 
  Serial.println("doing LMIC_setSession()...");
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  #endif

  // Set up the channels used by the Things Network, which correspond
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set. All g-band except the last
  // (which is g2-band):
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);

  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.
  // Disable link check validation
  LMIC_setLinkCheckMode(0);
  // set data rate and transmit power (note: txpow seems to be ignored by lib)
  LMIC_setDrTxpow(DR_SF7, 14);
  // start job
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
  
  // retrieve a point  
  TS_Point p = ts.getPoint();
  // scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
  p.y = map(p.y, TS_MAXY, TS_MINY, 0, tft.height());
    
  if(stage==1) {
    // respond to the touch
    if( // we're in the boxes
      (p.x >30 & p.x<100 & p.y >30 & p.y<100) |
      (p.x>220 & p.x<290 & p.y >30 & p.y<100) |
      (p.x >30 & p.x<100 & p.y >380 & p.y<450)|
      (p.x>220 & p.x<290 & p.y >380 & p.y<450)
    )
      tft.fillCircle(p.x, p.y, 20, HX8357_BLACK);
      
    if(p.x >105 & p.x<215 & p.y >315 & p.y<365) {
      stage=2;
      tft.fillScreen(HX8357_BLACK);
      tft.setCursor(60, 20);
      tft.setTextColor(HX8357_WHITE);
      tft.print("Accel Test");
      tft.fillRect(110, 120, 100, 40 , HX8357_WHITE);
      tft.setCursor(120, 130);
      tft.setTextColor(HX8357_BLACK);
      tft.print("NEXT!");
    }
  }

  if(stage==2) {
    sensors_event_t event;
    accel.getEvent(&event);
    tft.fillRect(0, 170, 319, 280 , HX8357_BLACK);
    tft.setCursor(0, 270);
    tft.setTextColor(HX8357_WHITE);
    tft.print("     X: "); tft.println(event.acceleration.x);
    tft.print("     Y: "); tft.println(event.acceleration.y);
    tft.print("     Z: "); tft.println(event.acceleration.z);
    delay(5);
    
    if(p.x >105 & p.x<215 & p.y >115 & p.y<165) {
      stage=3;
      tft.fillScreen(HX8357_WHITE);
      tft.setCursor(60, 20);
      tft.setTextColor(HX8357_BLACK);
      tft.print("Bat Man Test");
      tft.fillRect(110, 220, 100, 40 , HX8357_BLACK);
      tft.setCursor(120, 230);
      tft.setTextColor(HX8357_WHITE);
      tft.print("NEXT!");
    }
  }
  if(stage==3) {
    tft.setTextColor(HX8357_BLACK);
    tft.setCursor(0, 70);
    tft.print("  Version ");
    tft.println(getRegister(BM_I2Cadd, BM_Version));
    tft.println(" Expected 192");
    tft.print(" USB ");
    if(!powerGood)
      tft.print("un");
    tft.println("plugged");
    if(getRegister(BM_I2Cadd, BM_Version)!=192) {
      m.fail();
      tft.print("BATT MAN");
      delay(3000);
    }
    
    if(p.x >105 & p.x<215 & p.y >215 & p.y<265) {
      stage=4;
      tft.fillScreen(HX8357_BLACK);
      tft.fillRect(110, 320, 100, 40 , HX8357_WHITE);
      tft.setCursor(120, 330);
      tft.setTextColor(HX8357_BLACK);
      tft.print("NEXT!");
      tft.setCursor(80, 20);
      tft.setTextColor(HX8357_WHITE);
      tft.println("Mic Test");
    }
  }
  
  if(stage==4) {
    tft.fillRect(0, 160, 319, 90 , HX8357_BLACK);
    delay(5);
    uint32_t first = read_i2s(), second = read_i2s(), third = read_i2s();
    if(first != 32) Serial.println(first);
    if(second != 32) Serial.println(second);
    if(third != 32) Serial.println(third);
    tft.setCursor(70, 160);
    tft.println(first);
    tft.setCursor(70, 190);
    tft.println(second);
    tft.setCursor(70, 220);
    tft.println(third);
    if(first==0 & second == 0 & third ==0) {
      m.fail();
      tft.print("MICROPHONE");
      delay(3000);
    }
    if(p.x >105 & p.x<215 & p.y >315 & p.y<365) {
      stage=5;
      tft.fillScreen(HX8357_BLACK);
      tft.fillRect(110, 120, 100, 40 , HX8357_WHITE);
      tft.setCursor(120, 130);
      tft.setTextColor(HX8357_BLACK);
      tft.print("NEXT!");
      tft.setCursor(60, 20);
      tft.setTextColor(HX8357_WHITE);
      tft.println("IR LED Test");
    }
  }
  
  if(stage==5) {
    digitalWrite(12, HIGH);
    delay(100);
    digitalWrite(12, LOW);
    delay(100);
    if(p.x >105 & p.x<215 & p.y >115 & p.y<165) {
      stage=6;
      tft.fillScreen(HX8357_BLACK);
      tft.fillRect(110, 320, 100, 40 , HX8357_WHITE);
      tft.setCursor(120, 330);
      tft.setTextColor(HX8357_BLACK);
      tft.print("NEXT!");
      tft.setCursor(30, 20);
      tft.setTextColor(HX8357_WHITE);
      tft.println("SD Card Test");
      tft.setCursor(20, 120);
    }
  }

  if(stage==6) {
    IOExpander::digitalWrite(IOExpander::SD_CS, LOW);
    uint8_t cardType = SD.cardType();
    IOExpander::digitalWrite(IOExpander::SD_CS, HIGH);

    if(cardType == CARD_NONE) {
        D("No SD card attached");
        m.fail();
        tft.print("SD CARD");
        delay(3000);
    }
    tft.setCursor(50, 120);
    tft.println("SD Card Type: ");
    tft.setCursor(120, 190);
    if(cardType == CARD_MMC) {
        tft.println("MMC");
    } else if(cardType == CARD_SD) {
        tft.println("SDSC");
    } else if(cardType == CARD_SDHC) {
        tft.println("SDHC");
    } else {
        tft.println("UNKNOWN");
    }
    
    if(p.x >105 & p.x<215 & p.y >315 & p.y<365) {
      stage=7;
      tft.fillScreen(HX8357_BLACK);
      tft.fillRect(70, 120, 160, 40 , HX8357_WHITE);
      tft.setCursor(80, 130);
      tft.setTextColor(HX8357_BLUE);
      tft.print("STOP IT!");
      tft.setCursor(70, 20);
      tft.setTextColor(HX8357_WHITE);
      tft.println("Vibe Test");
    }
  }

  if(stage==7) {
    IOExpander::digitalWrite(IOExpander::VIBRATION, HIGH);
    delay(200);
    IOExpander::digitalWrite(IOExpander::VIBRATION, LOW);
    delay(200);
    if(p.x >105 & p.x<215 & p.y >115 & p.y<165) {
      stage=8;
      tft.fillScreen(HX8357_BLACK);
      tft.setCursor(60, 20);
      tft.setTextColor(HX8357_WHITE);
      tft.println("Audio Test");
    }
  }

  if(stage==8) {
    musicPlayer.sineTest(0x44, 2000); // make a tone to show VS1053 working
    if(p.x >105 & p.x<215 & p.y >115 & p.y<165) {
      stage=0;
      tft.fillRect(70, 220, 160, 40 , HX8357_GREEN);
      tft.setCursor(80, 230);
      tft.setTextColor(HX8357_BLACK);
      tft.print("ALL DONE!");
      tft.fillRect(40, 440, 140, 40 , HX8357_GREEN);
      tft.setCursor(50, 450);
      tft.setTextColor(HX8357_BLACK);
      tft.print("SWITCH!");
    }
  }
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
