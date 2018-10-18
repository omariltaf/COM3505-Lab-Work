// MyThing.ino
// COM3505 Lab Exercises Runner Sketch
// Omar Iltaf & Neville Kitala

#include "Week02.h"
#include "Week03.h"
#include "Week04.h"

int WEEKNUM = 4; // Enter the week number

// Initialisation entry point
void setup() {
  switch(WEEKNUM) {
    case  2: setup02(); break;
    case  3: setup03(); break;
    case  4: setup04(); break;
    default: Serial.println("Oops! Invalid week number");
  }
}

// Task loop entry point
void loop() {
  switch(WEEKNUM) {
    case  2: loop02(); break;
    case  3: loop03(); break;
    case  4: loop04(); break;
    default: Serial.println("Oops! Invalid week number");
  }
}
