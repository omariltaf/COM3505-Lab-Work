// Ex08.ino
// become a web client and send your email & the MAC to our cloud server
// DON'T edit these files, do your coding in MyThing!

const char* myEmail       = "fictional@sheffield.ac.uk";
String url;

void setup08() {
  setup07(); dln(startupDBG, "\nsetup08...");

  // a URI for the request
  url = "/com3505?email=";
  url += myEmail;
  url += "&mac=";
  url += MAC_ADDRESS;

  // conect to the guest network
  dbg(netDBG, "connecting to ");
  dln(netDBG, guestSsid);
  WiFi.begin(guestSsid, guestPassword);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    dbg(netDBG, ".");
  }

  dbg(netDBG, "wifi connected; IP address: ");
  dln(netDBG, WiFi.localIP());
} // setup08()

void loop08() {
  // serve pending web requests
  webServer.handleClient();

  // connect to the cloud server
  cloudConnect();

  // send the request to the server
  cloudGet(url);

  // read all the lines of the reply from server and print to serial
  while(cloudAvailable()) dbg(netDBG, cloudRead());

  // job's a goodun
  dln(netDBG, "closing connection and pausing...");
  cloudStop();
  delay(60000); // don't send too much data to the cloud server...
} // loop08()

// use WiFiClient class to create TCP connection to cloud
bool cloudConnect() {
  dbg(netDBG, "\nconnecting to ");
  dbg(netDBG, com3505Addr);
  dbg(netDBG, ":");
  dln(netDBG, com3505Port);
  if(com3505Client.connect(com3505Addr, com3505Port)) {
    dln(netDBG, "connected to com3505 server; doing GET");
  } else {
    dbg(netDBG, com3505Addr);
    dln(netDBG, " - no com3505 server");
    return false;
  }
  return true;
} // cloudConnect()

// do a GET request on com3505Client
void cloudGet(String url) {
  dbg(netDBG, "requesting URL: ");
  dln(netDBG, url);
  com3505Client.print(
    String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + com3505Addr + "\r\n" + "Connection: close\r\n\r\n"
  );
  unsigned long timeout = millis();
  while(com3505Client.available() == 0) {
    if(millis() - timeout > 5000) {
      dln(netDBG, ">>> client timeout !");
      cloudStop();
      return;
    }
  }
} // cloudGet()

// close cloud connection
void cloudStop() { com3505Client.stop(); }

// read a line from the cloud client after a request
String cloudRead() { return com3505Client.readStringUntil('\r'); }

// is there more to read from the response?
bool cloudAvailable() { return com3505Client.available(); }
