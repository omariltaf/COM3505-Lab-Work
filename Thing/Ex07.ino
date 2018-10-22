// Ex07.ino
// develop simple utilities for creating web pages

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

#include "Ex07.h"

void setup07() {
  setup06(); dln(startupDBG, "\nsetup07...");
}

void loop07() {
  webServer.handleClient(); // serve pending web requests every loop

  if(! (loopIteration++ % 500000 == 0)) // a slice every 0.5m iterations
    return;

  for(int i = 0; i < ALEN(boiler); i++) // print the boilerplate for reference
    dbg(miscDBG, boiler[i]);

  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, "a better title" },
    { 7, "Eat more green vegetables!" },
  };
  String htmlPage = ""; // a String to hold the resultant page
  getHtml(htmlPage, boiler, ALEN(boiler), repls, ALEN(repls)); // instantiate

  dbg(miscDBG, htmlPage.c_str()); // print the result
}

void getHtml( // turn array of strings & set of replacements into a String
  String& html, const char *boiler[], int boilerLen,
  replacement_t repls[], int replsLen
) {
  for(int i = 0, j = 0; i < boilerLen; i++) {
    if(j < replsLen && repls[j].position == i)
      html.concat(repls[j++].replacement);
    else
      html.concat(boiler[i]);
  }
}
