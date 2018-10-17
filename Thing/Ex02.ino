// Ex02.ino
// blinking an external LED; reading from a switch

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

void setup02() {
  setup01(); // include previous setup for serial, and for the internal LED
  Serial.printf("\nsetup02...\nESP32 MAC = %s\n", MAC_ADDRESS); // ESP's "ID"

  // set up GPIO pin for an external LED
  pinMode(32, OUTPUT); // set up pin 32 as a digital output

  // set up GPIO pin for a switch
  pinMode(14, INPUT_PULLUP); // pin 14: digital input, built-in pullup resistor
}

void loop02() {
  if(digitalRead(14) == LOW) { // switch pressed
    Serial.printf("switch is pressed...\n");
    useInternalLED = ! useInternalLED;
  }

  if(useInternalLED) {
    Serial.printf("blinking internal LED...\n");
    blink(1, 500);
  } else {
    Serial.printf("setting 32 HIGH...\n");
    digitalWrite(32, HIGH);       // on...
    delay(500);

    Serial.printf("setting 32 LOW...\n");
    digitalWrite(32, LOW);        // off...
    delay(500);
  }
}
