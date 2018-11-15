/////////////////////////////////////////////////////////////////////////////
// OTAThing.ino
// COM3505 lab assessment: Over-The-Air update model solution
// NOTE!!! DON'T edit this file, do your coding in MyOTAThing!
/////////////////////////////////////////////////////////////////////////////

/* 
This is the model solution for OTA update. It also includes the main
functionality up to Exercise 9 (starting an acces point and webserver,
allowing the user to choose a network to join, getting on-line, etc.)
*/

// the wifi and HTTP server libraries ///////////////////////////////////////
#include <WiFi.h>         // wifi
#include <ESPWebServer.h> // simple webserver
#include <HTTPClient.h>   // ESP32 library for making HTTP requests
#include <Update.h>       // OTA update library

// debugging infrastructure; setting different DBGs true triggers prints ////
#define dbg(b, s) if(b) Serial.print(s)
#define dln(b, s) if(b) Serial.println(s)
#define startupDBG      true
#define loopDBG         true
#define monitorDBG      true
#define netDBG          true
#define miscDBG         true
#define analogDBG       true
#define otaDBG          true

// OTA stuff ////////////////////////////////////////////////////////////////
int doCloudGet(HTTPClient *, String, String); // helper for downloading 'ware
void doOTAUpdate();                           // main OTA logic
int currentVersion = 3; // TODO keep up-to-date! (used to check for updates)
String gitID = "anotherexamplestudent"; // TODO change to your team's git ID

// MAC address //////////////////////////////////////////////////////////////
char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator
void getMAC(char *);

// LED utilities, loop slicing ///////////////////////////////////////////////
void ledOn();
void ledOff();
void blink(int = 1, int = 300);
int loopIteration = 0;

// globals for a wifi access point and webserver /////////////////////////////
String apSSID = String("OTAThing-");    // SSID of the AP
String apPassword = "dumbpassword";     // passkey for the AP; TODO change me!
ESPWebServer webServer(80);             // a simple web server
String ip2str(IPAddress);               // helper for printing IP addresses

// web server utils ////////////////////////////////////////////////////////
// the replacement_t type definition allows specification of a subset of the
// "boilerplate" strings, so we can e.g. replace only the title, or etc.
typedef struct { int position; const char *replacement; } replacement_t;
void getHtml(String& html, const char *[], int, replacement_t [], int);
// getting the length of an array in C can be complex...
// https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
#define ALEN(a) ((int) (sizeof(a) / sizeof(a[0]))) // only in definition scope!
#define GET_HTML(strout, boiler, repls) \
  getHtml(strout, boiler, ALEN(boiler), repls, ALEN(repls));

// SETUP: initialisation entry point ////////////////////////////////////////
void setup() {
  Serial.begin(115200);         // initialise the serial line
  getMAC(MAC_ADDRESS);          // store the MAC address
  Serial.printf("\nsetup...\nESP32 MAC = %s\n", MAC_ADDRESS);
  pinMode(BUILTIN_LED, OUTPUT); // set up GPIO pin for built-in LED
  pinMode(32, OUTPUT); // set up pin 32 as a digital output (external LED)
  pinMode(14, INPUT_PULLUP);    // pin 14: digital input, use pullup (switch)

  // the access point and network joining webserver stuff from Ex09
  blink(3);             // blink the on-board LED to say "hi"
  startAP();            // fire up the AP...
  initWebServer();      // ...and the web server
  blink(3);             // blink the on-board LED to say "bye"

  dln(startupDBG, "\nOTAThing setup..."); // debug printout
  Serial.printf("firmware is at version %d\n", currentVersion);

  // get on the network
  WiFi.begin("uos-other", "shefotherkey05"); // TODO register MAC first!
  uint8_t connectionTries = 0;
  Serial.print("connecting to wifi...");
  while (WiFi.status() != WL_CONNECTED) { // wait for a connection
    Serial.print(".");
    if(connectionTries++ % 100 == 0)   // print a line break for readability
      Serial.println("");
    else if(connectionTries % 10 == 0) // serve web requests, so that users...
      webServer.handleClient();        // ...can config wifi over the AP
    delay(250);
  }
  Serial.printf(
    "...connected; local IP is %s\n", ip2str(WiFi.localIP()).c_str()
  );

  // check for and perform firmware updates as needed
  doOTAUpdate();
}

// LOOP: task entry point ///////////////////////////////////////////////////
void loop() {
  int sliceSize = 500000;
  loopIteration++;
  if(loopIteration % sliceSize == 0) // a slice every sliceSize iterations
    dln(otaDBG, "OTA loop");

  webServer.handleClient(); // serve web requests every loop, like in Ex09
}

// OTA over-the-air update stuff ///////////////////////////////////////////
void doOTAUpdate() {             // the main OTA logic
  // materials for doing an HTTP GET on github from the BinFiles/ dir
  HTTPClient http; // manage the HTTP request process
  int respCode;    // the response code from the request (e.g. 404, 200, ...)
  int highestAvailableVersion = -1;  // version of latest firmware on server

  // do a GET to read the version file from the cloud
  Serial.println("checking for firmware updates...");
  respCode = doCloudGet(&http, gitID, "version");
  if(respCode == 200) // check response code (-ve on failure)
    highestAvailableVersion = atoi(http.getString().c_str());
  else
    Serial.printf("couldn't get version! rtn code: %d\n", respCode);
  http.end(); // free resources

  // do we know the latest version, and does the firmware need updating?
  if(respCode != 200) {
    Serial.printf("cannot update\n\n");
    return;
  } else if(currentVersion >= highestAvailableVersion) {
    Serial.printf("firmware is up to date\n\n");
    return;
  }

  // ok, we need to do a firmware update...
  Serial.printf(
    "upgrading firmware from version %d to version %d\n",
    currentVersion, highestAvailableVersion
  );

  // do a GET for the .bin
  String binName = String(highestAvailableVersion);
  binName += ".bin";
  respCode = doCloudGet(&http, gitID, binName);
  int updateLength = http.getSize(); // if isn't big enough refuse to update
  if(respCode == 200) {              // check response code (-ve on failure)
    Serial.printf(".bin code/size: %d; %d\n\n", respCode, updateLength);
    if(updateLength < 174992) {      // the size of the Blink example sketch
      Serial.println("update size is too small! refusing to try OTA update");
      return;
    }
  } else {
    Serial.printf("failed to get a .bin! return code is: %d\n", respCode);
    http.end(); // free resources
    return;
  }

  // write the new version of the firmware to flash
  WiFiClient stream = http.getStream();
  if(Update.begin(updateLength)) {
    Serial.printf("starting OTA may take a minute or two...\n");
    Update.writeStream(stream);
    if(Update.end()) {
      Serial.printf("update done, now finishing...\n");
      if(Update.isFinished()) {
        Serial.printf("update successfully finished; rebooting...\n\n");
        ESP.restart();
      } else {
        Serial.printf("update didn't finish correctly :(\n");
      }
    } else {
      Serial.printf("an update error occurred, #: %d\n" + Update.getError());
    }
  } else {
    Serial.printf("not enough space to start OTA update :(\n");
  }
  stream.flush();
}

// helper for downloading from cloud firmware server via HTTP GET
int doCloudGet(HTTPClient *http, String gitID, String fileName) {
  // build up URL from components; for example:
  // http://com3505.gate.ac.uk/repos/com3505-labs-2018-adalovelace/BinFiles/2.bin
  String baseUrl =
    "http://com3505.gate.ac.uk/repos/";
  String url =
    baseUrl + "com3505-labs-2018-" + gitID + "/BinFiles/" + fileName;

  // make GET request and return the response code
  http->begin(url);
  http->addHeader("User-Agent", "ESP32");
  return http->GET();
}

// misc utilities //////////////////////////////////////////////////////////
// get the ESP's MAC address
void getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion...
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
}

// LED blinkers
void ledOn()  { digitalWrite(BUILTIN_LED, HIGH); }
void ledOff() { digitalWrite(BUILTIN_LED, LOW); }
void blink(int times, int pause) {
  ledOff();
  for(int i=0; i<times; i++) {
    ledOn(); delay(pause); ledOff(); delay(pause);
  }
}

// access point utils ///////////////////////////////////////////////////////
void startAP() {
  apSSID.concat(MAC_ADDRESS);

  if(! WiFi.mode(WIFI_AP_STA))
    dln(startupDBG, "failed to set Wifi mode");
  if(! WiFi.softAP(apSSID.c_str(), apPassword.c_str()))
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

// web server utils /////////////////////////////////////////////////////////
void getHtml( // turn array of strings & set of replacements into a String
  String& html, const char *boiler[], int boilerLen,
  replacement_t repls[], int replsLen
) {
  for(int i = 0, j = 0; i < boilerLen; i++) {
    if(j < replsLen && repls[j].position == i)
      html.concat(repls[j++].replacement);
    else
      html.concat(boiler[i]);
  }
}
const char *templatePage[] = {    // we'll use Ex07 templating to build pages
  "<html><head><title>",                                                //  0
  "default title",                                                      //  1
  "</title>\n",                                                         //  2
  "<meta charset='utf-8'>",                                             //  3
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
  "<style>body{background:#FFF; color: #000; font-family: sans-serif;", //  4
  "font-size: 150%;}</style>\n",                                        //  5
  "</head><body>\n",                                                    //  6
  "<h2>Welcome to Thing!</h2>\n",                                       //  7
  "<!-- page payload goes here... -->\n",                               //  8
  "<!-- ...and/or here... -->\n",                                       //  9
  "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p>\n",                  // 10
  "</body></html>\n\n",                                                 // 11
};
void initWebServer() { // changed naming conventions to avoid clash with Ex06
  // register callbacks to handle different paths
  webServer.on("/", hndlRoot);              // slash
  webServer.onNotFound(hndlNotFound);       // 404s...
  webServer.on("/generate_204", hndlRoot);  // Android captive portal support
  webServer.on("/L0", hndlRoot);            // erm, is this...
  webServer.on("/L2", hndlRoot);            // ...IoS captive portal...
  webServer.on("/ALL", hndlRoot);           // ...stuff?
  webServer.on("/wifi", hndlWifi);          // page for choosing an AP
  webServer.on("/wifichz", hndlWifichz);    // landing page for AP form submit
  webServer.on("/status", hndlStatus);      // status check, e.g. IP address

  webServer.begin();
  dln(startupDBG, "HTTP server started");
}

// webserver handler callbacks
void hndlNotFound() {
  dbg(netDBG, "URI Not Found: ");
  dln(netDBG, webServer.uri());
  webServer.send(200, "text/plain", "URI Not Found");
}
void hndlRoot() {
  dln(netDBG, "serving page notionally at /");
  replacement_t repls[] = { // the elements to replace in the boilerplate
    {  1, apSSID.c_str() },
    {  8, "" },
    {  9, "<p>Choose a <a href=\"wifi\">wifi access point</a>.</p>" },
    { 10, "<p>Check <a href='/status'>wifi status</a>.</p>" },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07
  webServer.send(200, "text/html", htmlPage);
}
void hndlWifi() {
  dln(netDBG, "serving page at /wifi");

  String form = ""; // a form for choosing an access point and entering key
  apListForm(form);
  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, apSSID.c_str() },
    { 7, "<h2>Network configuration</h2>\n" },
    { 8, "" },
    { 9, form.c_str() },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07

  webServer.send(200, "text/html", htmlPage);
}
void hndlWifichz() {
  dln(netDBG, "serving page at /wifichz");

  String title = "<h2>Joining wifi network...</h2>";
  String message = "<p>Check <a href='/status'>wifi status</a>.</p>";

  String ssid = "";
  String key = "";
  for(uint8_t i = 0; i < webServer.args(); i++ ) {
    if(webServer.argName(i) == "ssid")
      ssid = webServer.arg(i);
    else if(webServer.argName(i) == "key")
      key = webServer.arg(i);
  }

  if(ssid == "") {
    message = "<h2>Ooops, no SSID...?</h2>\n<p>Looks like a bug :-(</p>";
  } else {
    char ssidchars[ssid.length()+1];
    char keychars[key.length()+1];
    ssid.toCharArray(ssidchars, ssid.length()+1);
    key.toCharArray(keychars, key.length()+1);
    WiFi.begin(ssidchars, keychars);
  }

  replacement_t repls[] = { // the elements to replace in the template
    { 1, apSSID.c_str() },
    { 7, title.c_str() },
    { 8, "" },
    { 9, message.c_str() },
  };
  String htmlPage = "";     // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls);

  webServer.send(200, "text/html", htmlPage);
}
void hndlStatus() {         // UI for checking connectivity etc.
  dln(netDBG, "serving page at /status");

  String s = "";
  s += "<ul>\n";
  s += "\n<li>SSID: ";
  s += WiFi.SSID();
  s += "</li>";
  s += "\n<li>Status: ";
  switch(WiFi.status()) {
    case WL_IDLE_STATUS:
      s += "WL_IDLE_STATUS</li>"; break;
    case WL_NO_SSID_AVAIL:
      s += "WL_NO_SSID_AVAIL</li>"; break;
    case WL_SCAN_COMPLETED:
      s += "WL_SCAN_COMPLETED</li>"; break;
    case WL_CONNECTED:
      s += "WL_CONNECTED</li>"; break;
    case WL_CONNECT_FAILED:
      s += "WL_CONNECT_FAILED</li>"; break;
    case WL_CONNECTION_LOST:
      s += "WL_CONNECTION_LOST</li>"; break;
    case WL_DISCONNECTED:
      s += "WL_DISCONNECTED</li>"; break;
    default:
      s += "unknown</li>";
  }

  s += "\n<li>Local IP: ";     s += ip2str(WiFi.localIP());
  s += "</li>\n";
  s += "\n<li>Soft AP IP: ";   s += ip2str(WiFi.softAPIP());
  s += "</li>\n";
  s += "\n<li>AP SSID name: "; s += apSSID;
  s += "</li>\n";

  s += "</ul></p>";

  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, apSSID.c_str() },
    { 7, "<h2>Status</h2>\n" },
    { 8, "" },
    { 9, s.c_str() },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07

  webServer.send(200, "text/html", htmlPage);
}
void apListForm(String& f) { // utility to create a form for choosing AP
  const char *checked = " checked";
  int n = WiFi.scanNetworks();
  dbg(netDBG, "scan done: ");

  if(n == 0) {
    dln(netDBG, "no networks found");
    f += "No wifi access points found :-( ";
    f += "<a href='/'>Back</a><br/><a href='/wifi'>Try again?</a></p>\n";
  } else {
    dbg(netDBG, n); dln(netDBG, " networks found");
    f += "<p>Wifi access points available:</p>\n"
         "<p><form method='POST' action='wifichz'> ";
    for(int i = 0; i < n; ++i) {
      f.concat("<input type='radio' name='ssid' value='");
      f.concat(WiFi.SSID(i));
      f.concat("'");
      f.concat(checked);
      f.concat(">");
      f.concat(WiFi.SSID(i));
      f.concat(" (");
      f.concat(WiFi.RSSI(i));
      f.concat(" dBm)");
      f.concat("<br/>\n");
      checked = "";
    }
    f += "<br/>Pass key: <input type='textarea' name='key'><br/><br/> ";
    f += "<input type='submit' value='Submit'></form></p>";
  }
}
String ip2str(IPAddress address) { // utility for printing IP addresses
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}
