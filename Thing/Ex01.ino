// Ex01.ino
// printing on the serial line; blinking the built-in LED

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

void setup01() {
  Serial.begin(115200);           // initialise the serial line
  getMAC(MAC_ADDRESS);            // store the MAC address as a chip identifier
  pinMode(BUILTIN_LED, OUTPUT);   // set up GPIO pin for built-in LED
  Serial.println("\nsetup01..."); // say hi
}
void loop01() {
  Serial.printf("ESP32 MAC = %s\n", MAC_ADDRESS); // print the ESP's "ID"
  blink(3);                     // blink the on-board LED...
  delay(1000);                  // ...and pause
}

// why not have getMAC return a String? we could, but see:
// https://hackingmajenkoblog.wordpress.com/2016/02/04/the-evils-of-arduino-strings/
// https://www.youtube.com/watch?v=eVGvPsCrQ_Y
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

void ledOn()  { digitalWrite(BUILTIN_LED, HIGH); }
void ledOff() { digitalWrite(BUILTIN_LED, LOW); }

void blink(int times, int pause) {
  ledOff();
  for(int i=0; i<times; i++) {
    ledOn(); delay(pause); ledOff(); delay(pause);
  }
}
