//  AllUIElement.h

#ifndef ALLUIELEMENT_H_
#define ALLUIELEMENT_H_

#include "UIController.h"
#include "ESPAudioFilePlayer.h"


//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of UIElement class. 
 */
class ConfigUIElement: public UIElement {
  private:
    long m_timer;
  public:
    ConfigUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { m_timer = millis(); };
    bool handleTouch(long x, long y);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of Touchpaint class. 
 */
class TouchpaintUIElement: public UIElement {
  private:
    void drawSelector();
    void colourSelector(long, long);
    uint16_t oldcolour;
    uint16_t currentcolour;
  public:
    TouchpaintUIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts) 
      : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of MusicPlayerUI class. 
 */
class MusicPlayerUIElement: public UIElement {
  private:
    ESPAudioFilePlayer *m_musicPlayer;
    bool m_once = true;
  public:
    MusicPlayerUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts) 
      : UIElement(tft, ts) { m_musicPlayer = new ESPAudioFilePlayer("/TRACK001.MP3", 1);};
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of TestCardUIElement class. 
 */
class TestCardUIElement: public UIElement {
  private:
    void bmpDraw(const char *filename, uint8_t x, uint16_t y);
    void drawBBC();
    void drawTestcard();
    uint16_t read16(File &f);
    uint32_t read32(File &f);
  public:
    TestCardUIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts) 
      : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of TextPageUIElement class. 
 */
class TextPageUIElement: public UIElement {
  private:
    void drawTextBoxes();
    uint8_t mapTextTouch(long, long);
    void printHistory(uint16_t x, uint16_t y);    
  public:
    TextPageUIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts) 
      : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of MicPlayerUIElement class. 
 */
class MicPlayerUIElement: public UIElement {
  private:
    void i2s_config();
    void i2s_read();
    long m_timer = 0; 
    bool once = true;
  public:
    MicPlayerUIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts) 
      : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of QwertyKeyboardUIElement class. 
 */
class QwertyKeyboardUIElement: public UIElement {
  private:
    void drawOutline();
    //void drawLetters();
    uint8_t mapKeyboardTouch(long, long);
  public:
    QwertyKeyboardUIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts) 
      : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};
//////////////////////////////////////////////////////////////////////////////

#endif
