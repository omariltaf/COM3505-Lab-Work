// MyProjectThing.ino
// COM3505 2018 project template sketch. Do your project code here.
// Out of the box the sketch is configured to kick the tyres on all the
// modules, and allow stepping through tests via the touch screen. Change the 
// TestScreen::activate(true); to false to change this behaviour.

#include "unphone.h"
#include <SPI.h>                 // the SPI bus
#include <Adafruit_GFX.h>        // core graphics library
#include "Adafruit_HX8357.h"     // tft display local hacked version
#include <Wire.h>                // I²C comms on the Arduino
#include <IOExpander.h>          // unPhone's IOExpander (controlled via I²C)
#include <GP2Y1010_DustSensor.h> // Library for the Sharp dust sensor
#include <DHTesp.h>              // Library for temperature / humidity sensor

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

// Air Quality sensor pins
const int GasPin = A0;
const int DustLEDPin = 32;
const int DustPin = 15;
const int dhtPin = 14;

DHTesp dht;
GP2Y1010_DustSensor dustsensor;
float screenHeight = 110.0f;

int screen = 1;
bool currentScreenIsDrawn = false;

// the LCD
#define TFT_DC   33      // this isn't on the IO expander

void setup() {
  Wire.setClock(100000); // higher rates trigger an IOExpander bug
  UNPHONE_DBG = true;
  Serial.begin(115200);  // init the serial line

  // fire up I²C, and the unPhone's IOExpander library
  Wire.begin();
  IOExpander::begin();

  checkPowerSwitch(); // check if power switch is now off & if so shutdown

  // which board version are we running?
  int version = IOExpander::getVersionNumber(), spin;
  if(version == 7) spin = 4;
  Serial.printf("starting, running on spin %d\n", spin);
  
  // show initial test screen on the LCD
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
  tft.begin(HX8357D);
  TestScreen::activate(false);
  TestScreen::init();
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);

  if(! ts.begin()) { // init the touchscreen
    D("failed to start touchscreen controller");
    TestScreen::fail("TOUCH");
    delay(3000);
  } else {
    D("touchscreen started");
  }
  
  if(!accel.begin()) // set up the accelerometer
  {
    D("Failed to start accelerometer");
    TestScreen::fail("ACCEL");
    delay(3000);
  }
  
  i2s_config(); // configure the I2S bus

  pinMode(IR_LEDS, OUTPUT); // IR_LED pin
  
  // set up the SD card
  IOExpander::digitalWrite(IOExpander::SD_CS, LOW);
  if(!SD.begin(-1)) {
    D("Card Mount Failed");
    TestScreen::fail("SD CARD");
    delay(3000);
  }
  IOExpander::digitalWrite(IOExpander::SD_CS, HIGH);

  if(! musicPlayer.begin()) { // initialise the music player
    D("Couldn't find VS1053, do you have the right pins defined?");
    TestScreen::fail("AUDIO");
    delay(3000);
  } else {
    D("VS1053 found");
  }
   
  // send a LoRaWAN message to TTN
  lmic_init();
  lmic_do_send(&sendjob);

  analogSetPinAttenuation(GasPin, ADC_11db);   // this sets ADC to 0-3.3V range
  analogSetPinAttenuation(DustPin, ADC_11db);
  dustsensor.begin(DustLEDPin, DustPin);
  dustsensor.setInputVolts(3.3);
  dustsensor.setADCbit(12);
  
  dht.setup(dhtPin, DHTesp::AM2302);
  Serial.println("DHT initiated");

  tft.fillScreen(HX8357_WHITE);
}

int prevX = 0;
int prevY = 0;
void loop() {
  bool usbPowerOn = checkPowerSwitch(); // shutdown if switch off
  if (!usbPowerOn) {
    return;
  }
  
  TS_Point p = ts.getPoint();
  p.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
  p.y = map(p.y, TS_MAXY, TS_MINY, 0, tft.height());
//  Serial.println(p.x);
//  if (p.x == prevX || p.y == prevY) {
//    return;
//  } else {
//    prevX = p.x;
//    prevY = p.y;
//  }
  
  if (screen == 1) {
    if (!currentScreenIsDrawn) {
      homeScreen();
      currentScreenIsDrawn = true;
    }
    if(p.x < 230 && p.x > 90 && p.y > 360 && p.y < 420) {
      screen = 2;
      currentScreenIsDrawn = false;
      Serial.println("lol");
    }
  } else if (screen == 2) {
    if (!currentScreenIsDrawn) {
      valueScreen();
      currentScreenIsDrawn = true;
    }
    drawValues();
    if(p.x < 230 && p.x > 90 && p.y > 360 && p.y < 420) {
      screen = 1;
      currentScreenIsDrawn = false;
      Serial.println("lol2");
    }
  }
}

void homeScreen() {
    tft.fillRect(0,0,320,200, HX8357_BLUE);
    tft.fillRect(0,200,320,100, HX8357_RED);
    tft.fillRect(90, 360, 140, 60 , HX8357_GREEN);
    
    tft.setTextSize(4);
    tft.setCursor(30, 80);
    tft.setTextColor(HX8357_WHITE);
    tft.print("Air Quality");
    tft.setCursor(80, 120);
    tft.print("Monitor");

    tft.setTextSize(2);
    tft.setCursor(115, 220);
    tft.print("Built by");
    tft.setCursor(90, 250);
    tft.print("Omar Iltaf &");
    tft.setCursor(80, 270);
    tft.print("Neville Kitala");

    tft.setTextSize(4);
    tft.setCursor(102, 375);
    tft.setTextColor(HX8357_WHITE);
    tft.print("BEGIN");
}

void valueScreen() {
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,200, HX8357_RED);
  tft.fillRect(90, 360, 140, 60 , HX8357_GREEN);
  
  tft.setTextSize(2.5);
  tft.setCursor(40, 50);
  tft.setTextColor(HX8357_WHITE);
  tft.print("Air Quality Overview");

  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(30, 130);
  tft.print("Dust Density");
  tft.setCursor(200, 130);
  tft.print(":");
  tft.setCursor(30, 170);
  tft.print("Gas Reading");
  tft.setCursor(200, 170);
  tft.print(":");
  tft.setCursor(30, 210);
  tft.print("Temperature");
  tft.setCursor(200, 210);
  tft.print(":");
  tft.setCursor(30, 250);
  tft.print("Humidity");
  tft.setCursor(200, 250);
  tft.print(":");

  tft.setTextSize(4);
  tft.setCursor(105, 375);
  tft.setTextColor(HX8357_WHITE);
  tft.print("BACK");
}

void drawValues() {
  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
  
  tft.setCursor(220,130);
  float dust = dustsensor.getDustDensity();
  tft.print(dust);
  Serial.println(dust);

  tft.setCursor(220,170);
  long valr = analogRead(GasPin);
  // the 3.3V range is divided into 4096 steps, giving 0.000806V per step
  // and the original voltage is divided by at 18K/10K resistor divider (*1.8)
  float volts = valr*0.000806*1.8;
  tft.print(String(volts) + "V");
  Serial.print(volts);
  Serial.println("V");
  
  TempAndHumidity newValues = dht.getTempAndHumidity();
  tft.setCursor(220,210);
  tft.print(String(newValues.temperature) + "C");
  tft.setCursor(220,250);
  tft.print(String(newValues.humidity) + "%RH");

  delay(1000);
}
