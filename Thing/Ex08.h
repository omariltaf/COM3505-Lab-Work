// Ex08.h
// DON'T edit these files, do your coding in MyThing!

#ifndef LAB_08_H 
#define LAB_08_H

// globals and utilities for connecting to the COM3505 cloud server via guest
WiFiClient com3505Client;  // the web client library class
const char *com3505Addr    = "com3505.gate.ac.uk";
const int   com3505Port    = 9191;
const char* guestSsid      = "uos-other";
const char* guestPassword  = "shefotherkey05";
bool cloudConnect();       // initialise com3505Client; true when connected
void cloudGet(String url); // do a GET on com3505Client
String cloudRead();        // read a line of response following a request
bool cloudAvailable();     // is there more to read from the response?
void cloudStop();          // shut connection on com3505Client

#endif
