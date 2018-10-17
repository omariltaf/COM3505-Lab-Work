// Ex04.ino
// debugging infrastructure

// setup and loop ///////////////////////////////////////////////////////////
void setup04() {
  setup03();            // previous setups: get the MAC, set up GPIO pins, ...
  dln(startupDBG, "\nsetup04..."); // debug printout on serial, with newline
}

void loop04() {
  dbg(loopDBG, "ESP32 MAC = "); // print the...
  dln(loopDBG, MAC_ADDRESS);    //   ...ESP's "ID"
  blink(3);                     // blink the on-board LED...
  delay(1000);                  //   ...and pause
}
