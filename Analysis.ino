#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFiClient.h>
#include <WiFi.h>

#define WLAN_SSID       "Neville"
#define WLAN_PASS       "neville1"


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "nevillekitala"
#define AIO_KEY         "ce6d1f76919149418106828f8b4bda7c"

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish compound = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/compound");
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish dust = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dust-sensor");


/*************************** Sketch Code ************************************/

void MQTT_connect();

// power management chip config
byte I2Cadd = 0x6b;      // I2C address of the PMU
byte BM_Watchdog = 0x05; // charge termination/timer control register
byte BM_OpCon    = 0x07; // misc operation control register
byte BM_Status   = 0x08; // system status register 
byte BM_Version  = 0x0a; // vender / part / revision status register 

// the LCD
#define TFT_DC   33      // this isn't on the IO expander

// the accelerometer (which we're using for attitude sensing)
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// setup ////////////////////////////////////////////////////////////////////
//void analysisInit(void) {
//  Serial.begin(115200);
//  Serial.println("Starting");
//  Serial.println("Version 1.01")
//
//  /* Initialise the sensor */
//  Wire.setClock(100000);
//  if(!accel.begin()) {
//    /* There was a problem detecting the sensor ... check your connections */
//    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
//    while(1);
//  }
//
//  WiFi.begin(WLAN_SSID, WLAN_PASS);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  
//  Serial.println("WiFi connected");
//  Serial.println("IP address: "); Serial.println(WiFi.localIP());
//  
//  IOExpander::begin();
//  checkPowerSwitch(); // check if the power switch is now off & if so shutdown
//    
//  tft.begin(HX8357D); // start the LCD driver chip
//  IOExpander::digitalWrite(IOExpander::BACKLIGHT,HIGH); // backlight on
//
//  
//}

void analysisLoop() {

  
  MQTT_connect();

  // Now we can publish stuff!
  Serial.print(F("\nSending humidity val "));
  Serial.print(rand);
  Serial.print("...");
  if (! humidity.publish(rand)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  Serial.print(F("\nSending compound val "));
  Serial.print(rand);
  Serial.print("...");
  if (! compound.publish(rand*1.1)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  Serial.print(F("\nSending dust val "));
  Serial.print(rand);
  Serial.print("...");
  if (! dust.publish(rand/2)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  Serial.print(F("\nSending temperature val "));
  Serial.print(rand);
  Serial.print("...");
  if (! temperature.publish(100 - rand)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
}


void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
