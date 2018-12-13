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
#include "WiFiStuff.h"

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

// Stored dust and gas sensor readings for use when connected to wifi
float dustVals[20];
float gasVals[20];

DHTesp dht;
GP2Y1010_DustSensor dustsensor;
float screenHeight = 110.0f;

int screen = 1;
bool currentScreenIsDrawn = false;
bool connectNetworkPressed = false;

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

  setSensorPins();
  
  dht.setup(dhtPin, DHTesp::AM2302);
  Serial.println("DHT initiated");

  tft.fillScreen(HX8357_WHITE);
}

void setSensorPins() {
  analogSetPinAttenuation(GasPin, ADC_11db);   // this sets ADC to 0-3.3V range
  analogSetPinAttenuation(DustPin, ADC_11db);
  dustsensor.begin(DustLEDPin, DustPin);
  dustsensor.setInputVolts(3.3);
  dustsensor.setADCbit(12);
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
  
  if (screen == 1) {
    if (!currentScreenIsDrawn) {
      homeScreen();
      currentScreenIsDrawn = true;
    }
    if(p.x < 230 && p.x > 90 && p.y > 300 && p.y < 360) {
      screen = 2;
      currentScreenIsDrawn = false;
    }
  } else if (screen == 2) {
    if (!currentScreenIsDrawn) {
      valueScreen();
      currentScreenIsDrawn = true;
    }
    drawValues();
    if(p.x < 140 && p.x > 0 && p.y > 360 && p.y < 420) {
      screen = 1;
      currentScreenIsDrawn = false;
    } else if (p.x < 320 && p.x > 180 && p.y > 360 && p.y < 420) {
      screen = 3;
      currentScreenIsDrawn = false;
    }
  } else if (screen == 3) {
    if (!currentScreenIsDrawn) {
      dustScreen();
      currentScreenIsDrawn = true;
    }
    dustScreenUpdate();
    if(p.x < 140 && p.x > 0 && p.y > 420 && p.y < 480) {
      screen = 2;
      currentScreenIsDrawn = false;
      resetValues();
    } else if (p.x < 320 && p.x > 180 && p.y > 420 && p.y < 480) {
      screen = 4;
      currentScreenIsDrawn = false;
      resetValues();
    }
  } else if (screen == 4) {
    if (!currentScreenIsDrawn) {
      gasScreen();
      currentScreenIsDrawn = true;
    }
    gasScreenUpdate();
    if(p.x < 140 && p.x > 0 && p.y > 360 && p.y < 420) {
      screen = 3;
      currentScreenIsDrawn = false;
      resetValues();
    } else if (p.x < 320 && p.x > 180 && p.y > 360 && p.y < 420) {
      screen = 5;
      currentScreenIsDrawn = false;
      resetValues();
    }
  } else if (screen == 5) {
    if (!currentScreenIsDrawn) {
      temperatureScreen();
      currentScreenIsDrawn = true;
    }
    temperatureScreenUpdate();
    if(p.x < 140 && p.x > 0 && p.y > 420 && p.y < 480) {
      screen = 4;
      currentScreenIsDrawn = false;
      resetValues();
    } else if (p.x < 320 && p.x > 180 && p.y > 420 && p.y < 480) {
      screen = 6;
      currentScreenIsDrawn = false;
      resetValues();
    }
  } else if (screen == 6) {
    if (!currentScreenIsDrawn) {
      humidityScreen();
      currentScreenIsDrawn = true;
    }
    humidityScreenUpdate();
    if(p.x < 140 && p.x > 0 && p.y > 360 && p.y < 420) {
      screen = 5;
      currentScreenIsDrawn = false;
      resetValues();
    } else if (p.x < 320 && p.x > 180 && p.y > 360 && p.y < 420) {
      screen = 7;
      currentScreenIsDrawn = false;
      resetValues();
    }
  } else if (screen == 7) {
    if (!currentScreenIsDrawn) {
      connectionScreen();
      currentScreenIsDrawn = true;
    }
    if(p.x < 140 && p.x > 0 && p.y > 420 && p.y < 480) {
      screen = 6;
      currentScreenIsDrawn = false; connectNetworkPressed = false; // disconnect from wifi here
    } else if (p.x < 320 && p.x > 180 && p.y > 420 && p.y < 480) {
      screen = 1;
      currentScreenIsDrawn = false; connectNetworkPressed = false; // disconnect wifi
    } else if (p.x < 260 && p.x > 60 && p.y > 160 && p.y < 280 && !connectNetworkPressed) {
      updateStatusMessage("Gathering sensor readings...");
      saveDustGasValues();
      delay(1000);
      updateStatusMessage("Connecting...");
      connectionInit();
      connectionLoop();
      connectNetworkPressed = true;
    }
  }
}

void homeScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,200, HX8357_BLUE);
  tft.fillRect(0,200,320,100, HX8357_RED);
  tft.fillRect(90, 300, 140, 60 , HX8357_GREEN);
    
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
  tft.setCursor(102, 320);
  tft.setTextColor(HX8357_WHITE);
  tft.print("BEGIN");
}

void valueScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,200, HX8357_RED);
  tft.fillRect(0, 360, 140, 60 , HX8357_GREEN);
  tft.fillRect(180, 360, 140, 60 , HX8357_GREEN);
  
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
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(20, 375);
  tft.print("BACK");
  tft.setCursor(200, 375);
  tft.print("NEXT");
}

float getDustValue() {
  float dust = dustsensor.getDustDensity();
  Serial.println(dust);
  return dust;
}

float getGasValue() {
  long valr = analogRead(GasPin);
  // the 3.3V range is divided into 4096 steps, giving 0.000806V per step
  // and the original voltage is divided by at 18K/10K resistor divider (*1.8)
  float volts = valr*0.000806*1.8;
  Serial.println(String(volts) + "V");
  return volts;
}

float getTemperatureValue() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
  Serial.println(String(newValues.temperature) + "C");
  return newValues.temperature;
}

float getHumidityValue() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
  Serial.println(String(newValues.humidity) + "%RH");
  return newValues.humidity;
}

void saveDustGasValues() {
  for (int i = 0; i < 20; i++) {
    dustVals[i] = getDustValue();
    gasVals[i] = getGasValue();
  }
}

void drawValues() {
  tft.fillRect(220,130,100,170, HX8357_RED);
  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
   
  tft.setCursor(220,130);
  float dust = getDustValue();
  tft.print(dust);

  tft.setCursor(220,170);
  long valr = analogRead(GasPin);
  // the 3.3V range is divided into 4096 steps, giving 0.000806V per step
  // and the original voltage is divided by at 18K/10K resistor divider (*1.8)
  float volts = getGasValue();
  tft.print(String(volts) + "V");
  
  tft.setCursor(220,210);
  tft.print(String(getTemperatureValue()) + "C");
  tft.setCursor(220,250);
  tft.print(String(getHumidityValue()) + "%RH");
  
  delay(200);
}

// x and y coords of the etching pen
int penx = 20;
int peny = 340;
int oldx = 20;
int oldy = 340;

void lineGraph(float current, String yLabel){
//  Serial.println(current);
  if(penx < 300 && penx > 20) {
    oldx = penx;
    oldy = peny;
    penx += 20;
    peny = 340 - current;
  }
  else {
    penx = 20;
    oldx = 20;
    oldy = peny;
    penx += 25;
    peny = 340 - current;
  
    tft.fillRect(0,100,320,260, HX8357_BLACK);
    tft.drawLine(20,340,300,340,HX8357_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(HX8357_WHITE);
    tft.setCursor(270,350);
    tft.println("Time (s)");
    
    tft.drawLine(20,340,20,120,HX8357_WHITE);
    tft.setCursor(0,110);
    tft.println(yLabel);

//    tft.drawLine(25,220,300,220,HX8357_RED);
//    tft.setCursor(270,230);
//    tft.println("Optimum");
  }
  tft.drawLine(oldx,oldy,penx, peny, HX8357_WHITE);
}

void resetValues() {
  penx = 20; oldx = 20;
  peny = 340; oldy = 340;
}

float scaleDustValue(float value) {
  return ((value-0)/(40-0))*(200-0)+0;
}

float scaleGasValue(float value) {
  return ((value-0)/(1-0))*(200-0)+0;
}

float scaleTemperatureValue(float value) {
  return ((value-0)/(50-0))*(200-0)+0;
}

float scaleHumidityValue(float value) {
  return ((value-0)/(100-0))*(200-0)+0;
}

void dustScreenUpdate() {
  tft.fillRect(220,0,100,100, HX8357_BLUE);
  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(220, 50);
  float dust = getDustValue();
  tft.print(String(dust));

  lineGraph(scaleDustValue(dust), "Dust Density");
  delay(200);
}

void dustScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,260, HX8357_BLACK);
  tft.fillRect(0, 420, 140, 60 , HX8357_GREEN);
  tft.fillRect(180, 420, 140, 60 , HX8357_GREEN);
  
  tft.setTextSize(2.5);
  tft.setCursor(30, 50);
  tft.setTextColor(HX8357_WHITE);
  tft.print("Dust Density");
  tft.setCursor(200, 50);
  tft.print(":");

  tft.setTextSize(4);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(20, 435);
  tft.print("BACK");
  tft.setCursor(200, 435);
  tft.print("NEXT");
}

void gasScreenUpdate() {
  tft.fillRect(220,0,100,100, HX8357_BLUE);
  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(220, 50);
  float gas = getGasValue();
  tft.print(String(gas) + "V");

  lineGraph(scaleGasValue(gas), "Gas Reading (V)");
  delay(200);
}

void gasScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,260, HX8357_BLACK);
  tft.fillRect(0, 360, 140, 60 , HX8357_GREEN);
  tft.fillRect(180, 360, 140, 60 , HX8357_GREEN);
  
  tft.setTextSize(2.5);
  tft.setCursor(30, 50);
  tft.setTextColor(HX8357_WHITE);
  tft.print("Gas Reading");
  tft.setCursor(200, 50);
  tft.print(":");

  tft.setTextSize(4);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(20, 375);
  tft.print("BACK");
  tft.setCursor(200, 375);
  tft.print("NEXT");
}

void temperatureScreenUpdate() {
  tft.fillRect(220,0,100,100, HX8357_BLUE);
  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(220, 50);
  float temperature = getTemperatureValue();
  tft.print(String(temperature) + "C");

  lineGraph(scaleTemperatureValue(temperature), "Temperature (C)");
  delay(200);
}

void temperatureScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,260, HX8357_BLACK);
  tft.fillRect(0, 420, 140, 60 , HX8357_GREEN);
  tft.fillRect(180, 420, 140, 60 , HX8357_GREEN);
  
  tft.setTextSize(2.5);
  tft.setCursor(30, 50);
  tft.setTextColor(HX8357_WHITE);
  tft.print("Temperature");
  tft.setCursor(200, 50);
  tft.print(":");

  tft.setTextSize(4);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(20, 435);
  tft.print("BACK");
  tft.setCursor(200, 435);
  tft.print("NEXT");
}

void humidityScreenUpdate() {
  tft.fillRect(220,0,100,100, HX8357_BLUE);
  tft.setTextSize(2.5);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(220, 50);
  float humidity = getHumidityValue();
  tft.print(String(humidity) + "%RH");

  lineGraph(scaleHumidityValue(humidity), "Humidity (%RH)");
  delay(200);
}

void humidityScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,260, HX8357_BLACK);
  tft.fillRect(0, 360, 140, 60 , HX8357_GREEN);
  tft.fillRect(180, 360, 140, 60 , HX8357_GREEN);
  
  tft.setTextSize(2.5);
  tft.setCursor(30, 50);
  tft.setTextColor(HX8357_WHITE);
  tft.print("Humidity");
  tft.setCursor(200, 50);
  tft.print(":");

  tft.setTextSize(4);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(20, 375);
  tft.print("BACK");
  tft.setCursor(200, 375);
  tft.print("NEXT");
}

void connectionScreen() {
  tft.fillScreen(HX8357_WHITE);
  tft.fillRect(0,0,320,100, HX8357_BLUE);
  tft.fillRect(0,100,320,260, HX8357_RED);
  tft.fillRect(60, 160, 200, 120 , HX8357_GREEN);
  tft.fillRect(0, 420, 140, 60 , HX8357_GREEN);
  tft.fillRect(180, 420, 140, 60 , HX8357_GREEN);
  
  tft.setTextSize(2.5);
  tft.setCursor(30, 50);
  tft.setTextColor(HX8357_WHITE);
  tft.print("Connect to Cloud");
  tft.setCursor(200, 50);
  tft.print(":");

  tft.setTextSize(1.5);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(20, 330);
  tft.print("Status: ");
  
  tft.setTextSize(2.5);
  tft.setCursor(100, 200);
  tft.print("Connect &");
  tft.setCursor(100, 230);
  tft.print("Send Data");
  
  tft.setTextSize(4);
  tft.setCursor(20, 435);
  tft.print("BACK");
  tft.setCursor(200, 435);
  tft.print("HOME");
}

void updateStatusMessage(String message) {
  tft.fillRect(80,330,270,30, HX8357_RED);
  tft.setTextSize(1);
  tft.setTextColor(HX8357_WHITE);
  tft.setCursor(80, 330);
  tft.print(message);
}
