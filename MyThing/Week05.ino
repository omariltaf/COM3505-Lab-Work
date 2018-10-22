// Week05.ino
// COM3505 Week 5 Sketch
// Ex08-Ex09
// Omar Iltaf & Neville Kitala

#include <WiFiClient.h>
#include "Week04.h"

const char* host = "com3505.gate.ac.uk";
const char* networks[20];
char buff[2046];
char HTMLstart[] = "<!DOCTYPE html>\n<html>\n<head>\n  <meta charset='utf-8'>\n</head>\n<body>\n<form>\n  Select Network: <select name='ssid'>\n ";
char HTMLpart1[] = " <option value='";
char HTMLpart2[] = "";
char HTMLpart3[] = "'>";
char HTMLpart4[] = "";
char HTMLpart5[] = "</option>\n ";
char HTMLend[] = "</select>\n  <br>Enter Password:\n  <input type='password' name='password'>\n  <br><input type='submit' value='Submit'>\n</form>\n</body>\n</html>\n";
int value = 0;

void setup05() {
  Serial.begin(115200);
  delay(10);
//  ex08Setup();
  ex09Setup();
}

void loop05() {
//  ex08();
  ex09();
}

// Ex08
void ex08Setup() {
  // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println("uos-other");

    WiFi.begin("uos-other", "shefotherkey05");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void ex08() {
  webServer.handleClient();
  
  delay(5000);
  ++value;
  
  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient myWiFiclient;
    const int httpPort = 9191;
    if (!myWiFiclient.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
//    String url = "/input/";
//    url += streamId;
//    url += "?private_key=";
//    url += privateKey;
//    url += "&value=";
//    url += value;

    String url = "/com3505?email=oiltaf1@sheffield.ac.uk&mac=30aea41b6b94";

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    myWiFiclient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (myWiFiclient.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            myWiFiclient.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(myWiFiclient.available()) {
        String line = myWiFiclient.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
}

// Ex09
void ex09Setup() {
      
    strcpy(buff, HTMLstart);
    getNetworks();
    strcat(buff, HTMLend);
    Serial.println(buff);
    
    createAP();
//    setup04();
}

void ex09() {
  webServer.handleClient();
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

void createAP() {
  webServer.on("/", handleRootForm);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  
  Serial.println(WiFi.softAPIP());              // Prints out the IP Address, SSID, and password for the access point
  //  Serial.println(ssid);
  //  Serial.println(password);
  
  webServer.begin(); 
}
