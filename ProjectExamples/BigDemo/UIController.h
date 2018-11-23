// UIController.h Created on: 22-06-2018, Author: @HarryEH

#ifndef UICONTROLLER_H_
#define UICONTROLLER_H_

#include <Adafruit_GFX.h>       // core graphics library
#include <Adafruit_HX8357.h>    // tft display
#include <Adafruit_STMPE610.h>  // touch screen
#include <SD.h>                 // the SD card
#include "unphone.h"
#include "foodflows.h"          // predictive text input
#include "IOExpander.h"

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of UIElement class. 
 */
class UIElement {
  protected:
    Adafruit_HX8357* m_tft;
    Adafruit_STMPE610* m_ts;

    // colour definitions
    const uint16_t BLACK =   HX8357_BLACK;
    const uint16_t BLUE =    HX8357_BLUE;
    const uint16_t RED =     HX8357_RED;
    const uint16_t GREEN =   HX8357_GREEN;
    const uint16_t CYAN =    HX8357_CYAN;
    const uint16_t MAGENTA = HX8357_MAGENTA;
    const uint16_t YELLOW =  HX8357_YELLOW;
    const uint16_t WHITE =   HX8357_WHITE;
    const uint8_t  BOXSIZE = 40;
    const uint8_t  HALFBOX = (BOXSIZE / 2);
    const uint8_t  QUARTBOX = (BOXSIZE / 4);
    const uint8_t  PENRADIUS = 9; // orig: 3
    static const uint8_t NUM_BOXES = 7;
    const uint16_t colour2box[NUM_BOXES] = {
      RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE,
    };
    const uint8_t SWITCHER = 7; // index of the switcher
    void drawSwitcher(uint16_t xOrigin = 0, uint16_t yOrigin = 0);
    
  public:
    UIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts);
    virtual bool handleTouch(long x, long y) = 0;
    virtual void draw() = 0;
    virtual void runEachTurn() = 0;
    void someFuncDummy();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of UIController class. 
 */
class UIController {
  private:
    UIElement* m_element;
    bool gotTouch();
    void handleTouch();
    void changeMode();
    void backlight(bool);
    void vibemotor(bool);
    ui_modes_t m_mode;

  public:
    UIController(ui_modes_t start_mode) { m_mode = start_mode;};
    bool begin();
    void run();
};
//////////////////////////////////////////////////////////////////////////////

#endif
