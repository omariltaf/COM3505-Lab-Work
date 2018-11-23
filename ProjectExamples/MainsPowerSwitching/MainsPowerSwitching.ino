#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);
  // Transmitter is connected to esp32 Pin #12
  mySwitch.enableTransmit(12);
  // We need to set pulse length as it's different from default
  mySwitch.setPulseLength(175);
}

void loop() {
  mySwitch.send(4281651, 24);      // lookup the code to match your socket
  Serial.println("Socket 1 On");
  delay(1000);  
  mySwitch.send(4281660, 24);      // these codes are for type 1406
  Serial.println("Socket 1 Off");
  delay(1000);
}
