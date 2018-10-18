// Week04.ino
// COM3505 Week 4 Sketch
// Ex06-Ex07
// Omar Iltaf & Neville Kitala

#include <WiFi.h>
#include <ESPWebServer.h>

const char* ssid = "omarneville";
const char* password = "password";
const char HTML[] = "<form onSubmit=\"event.preventDefault()\"><label class=\"label\">Network Name</label><input type=\"text\" name=\"ssid\"/><br/><label>Password</label><input type=\"text\" name=\"pass\"/><br/><input type=\"submit\" value=\"Submit\"></form>";

ESPWebServer webServer(80);

void handleRoot() {
  digitalWrite(LED_BUILTIN, 1);
  webServer.send(200, "text/html", HTML);
  digitalWrite(LED_BUILTIN, 0);
}

void setup04() {
  Serial.begin(115200);
  
  webServer.on("/", handleRoot);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  
  Serial.println(WiFi.softAPIP());              // Prints out the IP Address, SSID, and password for the access point
  //  Serial.println(ssid);
  //  Serial.println(password);
  
  webServer.begin();  
}

void loop04() {
  webServer.handleClient();
}
