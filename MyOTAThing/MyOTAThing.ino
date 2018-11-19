/////////////////////////////////////////////////////////////////////////////
// MyOTAThing.ino
// COM3505 lab assessment: Over-The-Air update template; ADD YOUR CODE HERE!
/////////////////////////////////////////////////////////////////////////////

// the wifi and HTTP server libraries ///////////////////////////////////////
#include <WiFi.h>         // wifi
#include <ESPWebServer.h> // simple webserver
#include <HTTPClient.h>   // ESP32 library for making HTTP requests
#include <Update.h>       // OTA update library

// OTA stuff ////////////////////////////////////////////////////////////////
int doCloudGet(HTTPClient *, String, String); // helper for downloading 'ware
void doOTAUpdate();                           // main OTA logic
int currentVersion = 20; // TODO keep up-to-date! (used to check for updates)
String gitID = "omariltaf"; // TODO change to your team's git ID
String host = "com3505.gate.ac.uk";
bool updateSuccess;

// MAC and IP helpers ///////////////////////////////////////////////////////
char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator
void getMAC(char *);
String ip2str(IPAddress);                 // helper for printing IP addresses

// LED utilities, loop slicing ///////////////////////////////////////////////
void ledOn();
void ledOff();
void blink(int = 1, int = 300);
int loopIteration = 0;

// WiFi Details ///////////////////////////////////////////////
const char* ssid = "WeeFee";
const char* password = "password";

// SETUP: initialisation entry point ////////////////////////////////////////
void setup() {
  Serial.begin(115200);         // initialise the serial line
  Serial.println("**********************************************************");
  getMAC(MAC_ADDRESS);          // store the MAC address
  Serial.printf("\nMyOTAThing setup...\nESP32 MAC = %s\n", MAC_ADDRESS);
  Serial.printf("firmware is at version %d\n", currentVersion);

  // get on the network
  /* TODO
  Here you need to use the WiFi class to join the uos-other network, e.g. by
  calling:
      WiFi.begin("uos-other", "shefotherkey05"); // TODO register MAC first!
  You'll then need to wait until the wifi status is WL_CONNECTED
  */
  connect();

  // check for and perform firmware updates as needed
  doOTAUpdate();
}

// LOOP: task entry point ///////////////////////////////////////////////////
void loop() {
  int sliceSize = 500000;
  loopIteration++;
  if(loopIteration % sliceSize == 0) { // a slice every sliceSize iterations
//    Serial.println("OTA loop");
    Serial.println("I am version " + String(currentVersion));
  }

  // do other useful stuff here...?
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
  /* TODO
  Your main OTA update logic goes here. Use doCloudGet to do an HTTP GET for
  the .bin file, and check that the length of the update is appropriate (via
  HTTPClient's getSize() method. (You might note that a minimal sketch takes
  more than 100k bytes, so perhaps that is a good length to check for. Check
  the response code, and if all is well call Update.begin (which takes the
  size of the update as parameter) and Update.writeStream (which takes a
  Stream that you can get from HTTPClient's getStream() method). Other methods
  needed are Update.end, Update.isFinished and Update.getError. When an update
  has been performed correctly, you can restart the device via ESP.restart().
  */
  updateSuccess = false;
  // attempts to get the update 3 times before running the current firmware
  for(int i = 0; i < 3; i++) {
    if (!updateSuccess) {
      Serial.println("Update attempt #" + String(i + 1));
      getBin(highestAvailableVersion);
      delay(1000);
    }
  }
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

// get bin file from specified cloud server
void getBin(int highestAvailableVersion) {
  HTTPClient http; // manage the HTTP request process
  int respCode;    // the response code from the request (e.g. 404, 200, ...)

  // do a GET to read the version file from the cloud
  Serial.println("checking for firmware updates...");
  // form the appropriate filename
  String fileName = String(highestAvailableVersion) + ".bin";
  respCode = doCloudGet(&http, gitID, fileName);
  if(respCode == 200) { // check response code (-ve on failure)
    WiFiClient httpStream = http.getStream();
    // get the size of the file
    int contentLength = http.getSize();
    Serial.println(String(contentLength));
    if (contentLength) {
      // Check if there is enough space to perform OTA Update
      bool canBegin = Update.begin(contentLength);
      
      // If yes, begin
      if (canBegin) {
        Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
        // No activity would appear on the Serial monitor
        // So be patient. This may take 2 - 5mins to complete
        size_t written = Update.writeStream(httpStream);
        // checks if complete file has been written to device
        if (written == contentLength) {
          Serial.println("Written : " + String(written) + " successfully");
          } else {
            Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
            // retry??
            // execOTA();
            }
      // checks if update process is successful and if so, reboots device
      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          updateSuccess = true;
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
          updateSuccess = false;
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        updateSuccess = false;
      }
    } else {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println("Not enough space to begin OTA");
      httpStream.flush();
    }
    }
  }
  else
    Serial.printf("couldn't get bin file! rtn code: %d\n", respCode);
  http.end(); // free resources
  
  if(respCode != 200) {
    Serial.printf("cannot get bin file\n\n");
    return;
  }
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

// utility for printing IP addresses
String ip2str(IPAddress address) {
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}

// connect to WiFi
void connect() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
