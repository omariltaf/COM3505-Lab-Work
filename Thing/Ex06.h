// Ex06.h

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

#ifndef LAB_06_H 
#define LAB_06_H

// the wifi and HTTP server libraries ////////////////////////////////////////
#include <WiFi.h>
#include <ESPWebServer.h>

// globals for a wifi access point and webserver /////////////////////////////
String apSSID;                  // SSID of the AP
ESPWebServer webServer(80);     // a simple web server

#endif
