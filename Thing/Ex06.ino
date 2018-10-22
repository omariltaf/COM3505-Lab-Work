// Ex06.ino
// starting up a wifi access point and web server

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

// setup and loop ///////////////////////////////////////////////////////////
void setup06() {
  setup05();            // previous setups...
  dln(startupDBG, "\nsetup06..."); // debug printout on serial, with newline

  startAP();            // fire up the AP...
  startWebServer();     // ...and the web server
}
void loop06() {
  // things to try: how responsive is it with different time slicing options?
  // or what if we add in "loop02();"...?
  // what happens when there are mulitple requests?
  // how might this compare with responsiveness on the ESP8266?

  if(! (loopIteration++ % 1000 == 0)) // a loop slice every thousand iterations
    return;

  dln(netDBG, "calling webServer.handleClient()...");
  webServer.handleClient(); // deal with any pending web requests
}

// startup utilities ////////////////////////////////////////////////////////
void startAP() {
  apSSID = String("Thing-");
  apSSID.concat(MAC_ADDRESS);

  if(! WiFi.mode(WIFI_AP_STA))
    dln(startupDBG, "failed to set Wifi mode");
  if(! WiFi.softAP(apSSID.c_str(), "dumbpassword"))
    dln(startupDBG, "failed to start soft AP");
  printIPs();
}
void printIPs() {
  if(startupDBG) { // easier than the debug macros for multiple lines etc.
    Serial.print("AP SSID: ");
    Serial.print(apSSID);
    Serial.print("; IP address(es): local=");
    Serial.print(WiFi.localIP());
    Serial.print("; AP=");
    Serial.println(WiFi.softAPIP());
  }
  if(netDBG)
    WiFi.printDiag(Serial);
}
void startWebServer() {
  // register callbacks to handle different paths
  webServer.on("/", handleRoot);
  webServer.on("/hello", handleHello);

  // 404s...
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  dln(startupDBG, "HTTP server started");
}

// HTML page creation utilities //////////////////////////////////////////////
// (we'll see a better version in Ex07!)
String getPageTop() {
  return
    "<html><head><title>COM3506 IoT [ID: " + apSSID + "]</title>\n"
    "<meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "\n<style>body{background:#FFF; color: #000; "
    "font-family: sans-serif; font-size: 150%;}</style>\n"
    "</head><body>\n"
  ;
};
String getPageBody() {
  return "<h2>Welcome to Thing!</h2>\n";
}
String getPageFooter() {
  return "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p></body></html>\n";
}

// webserver handler callbacks ///////////////////////////////////////////////
void handleNotFound() {
  dbg(netDBG, "URI Not Found: ");
  dln(netDBG, webServer.uri());
  webServer.send(200, "text/plain", "URI Not Found");
}
void handleRoot() {
  dln(netDBG, "serving page notionally at /");
  String toSend = getPageTop();
  toSend += getPageBody();
  toSend += getPageFooter();
  webServer.send(200, "text/html", toSend);
}
void handleHello() {
  dln(netDBG, "serving /hello");
  webServer.send(
    200,
    "text/plain",
    "Hello! Have you considered sending your lecturer a large gift today? :)\n"
  );
}
