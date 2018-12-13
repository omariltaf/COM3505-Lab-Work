// Omar Iltaf & Neville Kitala

#include <WiFiClient.h>
#include <WiFi.h>
#include <ESPWebServer.h>
//#include "Analysis.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

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


ESPWebServer webServer(80);
const char* ssid = "omarneville";
const char* password = "password";
const char* host = "com3505.gate.ac.uk";
const char* networks[20];
char buff[2046];
char HTMLstart[] = "<!DOCTYPE html>\n<html>\n<head>\n  <meta charset='utf-8'>\n</head>\n<body>\n<form action='http://192.168.4.1/submit' method = 'POST'>\n  Select Network: <select name='ssid'>\n ";
char HTMLpart1[] = " <option value='";
char HTMLpart2[] = "";
char HTMLpart3[] = "'>";
char HTMLpart4[] = "";
char HTMLpart5[] = "</option>\n ";
char HTMLend[] = "</select>\n  <br>Enter Password:\n  <input type='password' name='password'>\n  <br>Enter Adafruit.io Username:\n  <input type='text' name='adauser'>\n <br><input type='submit' value='Submit'>\n</form>\n</body>\n</html>\n";
int value = 0;

void MQTT_connect();

void connectionInit() {
      
    strcpy(buff, HTMLstart);
    getNetworks();
    strcat(buff, HTMLend);
    Serial.println(buff);
    
    createAP();
}

void connectionLoop() {
  Serial.println("looooooooooooool1111111");
  while(true) {
    Serial.println("looooooooooooool222222");
    webServer.handleClient();
  }
}

void getNetworks() {
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("Setup done");
  
  Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
//          bool isDuplicate = false;
//          for (int j = 0; j < i; j++) {
//            if (WiFi.SSID(j) == WiFi.SSID(i)) {
//              isDuplicate = true;
//              break;
//            }
//          }
//          if (isDuplicate) {
//            
//          }
          networks[i] = (WiFi.SSID(i)).c_str();
            Serial.println(networks[i]);
            strcat(buff, HTMLpart1);
            strcat(buff, networks[i]);
            strcat(buff, HTMLpart3);
            strcat(buff, networks[i]);
            strcat(buff, HTMLpart5);
            
          delay(10);
        }
    }
    Serial.println("");

    // Wait a bit before scanning again
    delay(5000);
}

void handleRootForm() {
  digitalWrite(LED_BUILTIN, 1);
  webServer.send(200, "text/html", buff);
  digitalWrite(LED_BUILTIN, 0);
}

void handleSubmit() {
  if (webServer.hasArg("password")== false){ //Check if body received
 
            webServer.send(200, "text/plain", "Body not received");
            return;
 
      }
 
      String message = "connection made to:\n";
             message += webServer.arg("ssid");
             message += "\n";

             char a[20];
             (webServer.arg("ssid")).toCharArray(a, 20);
             char b[20];
             (webServer.arg("password")).toCharArray(b, 20);
             char c[20];
             (webServer.arg("adauser")).toCharArray(c, 20);
             
    
 
      webServer.send(200, "text/plain", message);
      
// We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(webServer.arg(a));

    WiFi.begin(a, b);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    Serial.println(message);
    Serial.println("Now connecting to Adafruit.io using supplied credentials...");
//    analysisInit();
//    while (true) {
//      analysisLoop();
//    }

    for(int i = 0; i < 20; i++) {
      analysisLoop(i);
      delay(10000);
    }
}

void createAP() {
  webServer.on("/", handleRootForm);
  webServer.on("/submit", handleSubmit);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  
  Serial.println(WiFi.softAPIP());              // Prints out the IP Address, SSID, and password for the access point
  //  Serial.println(ssid);
  //  Serial.println(password);
  
  webServer.begin(); 
}

void analysisLoop(int index) {
  
  float temperatureVal = getTemperatureValue();
  float humidityVal = getHumidityValue();
  MQTT_connect();

  // Now we can publish stuff!
  Serial.print(F("\nSending humidity val "));
  Serial.print(humidityVal);
  Serial.print("...");
  if (! humidity.publish(humidityVal)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  Serial.print(F("\nSending compound val "));
  Serial.print(gasVals[index]);
  Serial.print("...");
  if (! compound.publish(gasVals[index])) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  Serial.print(F("\nSending dust val "));
  Serial.print(dustVals[index]);
  Serial.print("...");
  if (! dust.publish(dustVals[index])) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  Serial.print(F("\nSending temperature val "));
  Serial.print(temperatureVal);
  Serial.print("...");
  if (! temperature.publish(temperatureVal)) {
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
