// ConfigUIElement.cpp

#include "AllUIElement.h"

#include <WiFi.h>

//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool ConfigUIElement::handleTouch(long x, long y) {
  return y < BOXSIZE && x > (BOXSIZE * SWITCHER);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Function that controls the drawing on the config page
 * Writes various things including the temperature, mac address and
 * the wifi ssid. If connected to wifi it will show the number of people 
 * in the diamond taken from the RESTful API at
 * http://www.canistudy.com/diamond
 */
void ConfigUIElement::draw(){

  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);

  m_tft->setCursor(0, 0);
  m_tft->print("[config mode]");
  drawSwitcher();

  // display the on-board temperature
  float onBoardTemp = temperatureRead();
  char buf[32];
  sprintf(buf, "temp: %.2f C", onBoardTemp);
  m_tft->setCursor(0, 20);
  m_tft->print(buf);

  // Display the on-board mac address.
  char mac_buf[13];
  m_tft->setCursor(0, 40);
  m_tft->print("MAC: ");
  m_tft->print(getMAC(mac_buf));

  // Display the WiFi Characteristics
  m_tft->setCursor(0, 60);
  m_tft->print(WiFi.SSID());

  m_tft->setCursor(0, 80);
  
  if (WiFi.status() == WL_CONNECTED) {
    m_tft->print("Connected to WiFi ^^");
  } else {
    m_tft->print("Not connected to WiFi ^^");
  }

  // i2s_config();

  // printf("hello\n");

}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Function thats runs each turn. Currently does nothing
 */
void ConfigUIElement::runEachTurn(){
  
}
//////////////////////////////////////////////////////////////////////////
