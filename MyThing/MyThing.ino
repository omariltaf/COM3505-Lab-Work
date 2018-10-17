// MyThing.ino
// COM3505 Week 2 lab exercises sketch
// Omar Iltaf & Neville Kitala

#define DEBUG_MACRO 1

char MAC_ADDRESS[13];
int counter = 0;
int buttonState = 0;                    // Reads the pushbutton status

// Pin Numbers
int pushButton = 14;
int externalRedLED = 32;
int externalGreenLED = 12;
int externalAmberLED = 15;

// The setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);                 // Initialise the serial line
  getMAC(MAC_ADDRESS);                  // Store the MAC address as a chip identifier
  pinMode(pushButton, INPUT_PULLUP);    // Set up GPIO pins for the switch and LEDs 
  pinMode(externalRedLED, OUTPUT);
  pinMode(externalGreenLED, OUTPUT);
  pinMode(externalAmberLED, OUTPUT);
}

// The loop function runs over and over again forever
void loop() {
  counter++;                                           // Increments loop count
  Serial.printf("ESP32 MAC = %s\n", MAC_ADDRESS);      // Prints the ESP's "ID"
  buttonState = digitalRead(pushButton);               // Reads the state of the pushbutton value
  
  #if DEBUG_MACRO                    // Ex04 - Prints out loop counter value for debugging
  Serial.println(counter);
  #endif

  if (counter % 10 == 0) {              // Does something special every ten iterations
    everyTenIterations(5000);
  } else {
    if (buttonState == LOW) {           // Checks if the pushbutton is pressed (buttonState is LOW)
//      blink(3, 300);
      trafficLights(600);
    }
  }
  
  delay(1000);                        // Waits for 1 second
}

// Stores the ESP's MAC Address within the passed in character array
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

// Helper functions to operate the LEDs
void ledOn(int ledPin)  { digitalWrite(ledPin, HIGH); }
void ledOff(int ledPin) { digitalWrite(ledPin, LOW); }

// Ex02 - Blinks the Red LED
void blink(int times, int pause) {
  ledOff(externalRedLED);
  for(int i=0; i<times; i++) {
    ledOn(externalRedLED); delay(pause); ledOff(externalRedLED); delay(pause);
  }
}

// Ex03 - Initiates a single traffic light sequence
void trafficLights(int pause) {
  ledOff(externalRedLED); ledOff(externalGreenLED); ledOff(externalAmberLED);
  ledOn(externalRedLED); delay(pause);
  ledOn(externalAmberLED); delay(pause);
  ledOff(externalRedLED); ledOff(externalAmberLED); ledOn(externalGreenLED); delay(pause);
  ledOff(externalGreenLED); ledOn(externalAmberLED); delay(pause);
  ledOff(externalAmberLED);
}

// Ex05 - Turns on all LEDs every ten iterations
void everyTenIterations(int pause) {
  ledOn(externalRedLED); ledOn(externalGreenLED); ledOn(externalAmberLED); delay(pause);
    ledOff(externalRedLED); ledOff(externalGreenLED); ledOff(externalAmberLED);
}
