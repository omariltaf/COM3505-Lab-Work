// MyThing.ino
// COM3505 Week 2 lab exercises sketch
// Omar Iltaf & Neville Kitala

#include <WiFi.h>
#include <ESPWebServer.h>

const char* ssid = "omarneville";
const char* password = "password";

ESPWebServer webServer(80);

void handleRoot() {
  digitalWrite(LED_BUILTIN, 1);
  webServer.send(200, "text/plain", "hello from esp!");
  digitalWrite(LED_BUILTIN, 0);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);

  //Print the server name and password and local IP
  
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(WiFi.localIP());

  webServer.on("/", handleRoot);

  webServer.begin();
  Serial.println("HTTP server started");
  Serial.println("<!DOCTYPE html><html>");
  Serial.println("<body><h1>ESP32 Web Server</h1>");
  Serial.println("</body></html>");


}

void loop() {
  webServer.handleClient();
}

