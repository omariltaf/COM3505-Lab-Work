// MyProjectThing.ino
// COM3505 2018 project template sketch. Do your project code here.
// Out of the box the sketch is configured to kick the tyres on all the
// modules, and allow stepping through tests via the touch screen. Change the 
// TestScreen::activate(true); to false to change this behaviour.

#include "unphone.h"

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
  TestScreen::activate(true);
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
}

void loop() {
  bool usbPowerOn = checkPowerSwitch(); // shutdown if switch off
  
  TestScreen::testSequence(usbPowerOn); // run a test on all modules
}
