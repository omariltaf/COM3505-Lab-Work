// Week02.ino
// COM3505 Week 2 Sketch
// Ex01
// Omar Iltaf & Neville Kitala

uint64_t chipid;

// The setup function runs once when you press reset or power the board
void setup02() {
  // Initialise digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

// The loop function runs over and over again forever
void loop02() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

//  Alternative String processing
//  String s = "ESP32 Chip ID = ";
//  s.concat((uint16_t)(chipid>>32));
//  s.concat((uint32_t)chipid);
//  s.concat("\n");
//  Serial.println(s);
  
  delay(1000);                      // wait for a second
}
