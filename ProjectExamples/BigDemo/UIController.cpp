// UIController.cpp

#include "AllUIElement.h"

/////////////////////////////////////////////////////////////////////////
// bitmap drawing (specific to the HX8357
void bmpDraw(const char *filename, uint8_t x, uint16_t y);
uint16_t read16(File &f);
uint32_t read32(File &f);

// instantiate the display and touch screen
Adafruit_HX8357 tft =
  Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);
Adafruit_STMPE610 ts = Adafruit_STMPE610(IOExpander::TOUCH_CS);

// keep Arduino IDE compiler happy
UIElement::UIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts) { 
  m_tft = tft;
  m_ts = ts; 
}
void UIElement::someFuncDummy() { }

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
bool UIController::begin() {
  D("UI controller started\n")

  IOExpander::begin();

  D("UI.begin()\n")
  bool status = ts.begin();
  if(!status) {
    E("failed to start touchscreen controller\n");
  } else {
    D("worked\n");
  }

  // turn on the backlight
  // pinMode(2, OUTPUT);
  // backlight(true);
  // TODO vibemotor(false);
  /* TODO
  ts.writeRegister8(0x17, 12);  // use GPIO2 & 3 (bcd)
  ts.writeRegister8(0x13, 252); // all gpio's as out to minimise pwr on unused
  ts.writeRegister8(0x11, 4);   // set GPIO2 LOW to disable vibe
  ts.writeRegister8(0x10, 8);   // and GPIO3 HIGH to enable backlight 
  */

  D("hello world\n");
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
  tft.begin(HX8357D);
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);
  D("TFT started\n");

  IOExpander::digitalWrite(IOExpander::SD_CS, LOW); // TODO shouldn't be needed
  if (!SD.begin(-1)) {
    E("failed!\n")
  } else {
    D("OK!\n");
  } 
  IOExpander::digitalWrite(IOExpander::SD_CS, HIGH);

  tft.fillScreen(HX8357_GREEN); WAIT_MS(50) tft.fillScreen(HX8357_BLACK);
  
  // define the first m_element here 
  switch(m_mode) {
    case ui_qwerty:
      m_element = new QwertyKeyboardUIElement(&tft, &ts);  break;
    case ui_touchpaint:
      m_element = new TouchpaintUIElement(&tft, &ts);      break;
    case ui_configure:
      m_element = new ConfigUIElement(&tft, &ts);          break;
    case ui_text:
      m_element = new TextPageUIElement(&tft, &ts);        break;
    case ui_testcard:
      m_element = new TestCardUIElement(&tft, &ts);        break;
    case ui_mic:
      m_element = new MicPlayerUIElement(&tft, &ts);       break;
    case ui_music:
      m_element = new MusicPlayerUIElement(&tft, &ts);     break;
    default:
      m_element = new TextPageUIElement(&tft, &ts);
  }

  m_element->draw();

  return status;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Touch management code. This is unchanged.//////////////////////////////
TS_Point nowhere(-1, -1, -1);    // undefined coordinate
TS_Point firstTouch(0, 0, 0);    // the first touch defaults to 0,0,0
TS_Point p(-1, -1, -1);          // current point of interest (signal)
TS_Point prevSig(-1, -1, -1);    // the previous accepted touch signal
bool firstTimeThrough = true;    // first time through gotTouch() flag
uint16_t fromPrevSig = 0;        // distance from previous signal
unsigned long now = 0;           // millis
unsigned long prevSigMillis = 0; // previous signal acceptance time
unsigned long sincePrevSig = 0;  // time since previous signal acceptance
uint16_t DEFAULT_TIME_SENSITIVITY = 150; // min millis between touches
uint16_t TIME_SENSITIVITY = DEFAULT_TIME_SENSITIVITY;
uint16_t DEFAULT_DIST_SENSITIVITY = 200; // min distance between touches
uint16_t DIST_SENSITIVITY = DEFAULT_DIST_SENSITIVITY;
uint16_t TREAT_AS_NEW = 600;     // if no signal in this period treat as new
uint8_t modeChangeRequests = 0;  // number of requests to switch mode

void setTimeSensitivity(uint16_t s = DEFAULT_TIME_SENSITIVITY) {
  TIME_SENSITIVITY = s;
}

void setDistSensitivity(uint16_t d = DEFAULT_DIST_SENSITIVITY) {
  DIST_SENSITIVITY = d;
}

uint16_t distanceBetween(TS_Point a, TS_Point b) { // coord distance /////
  uint32_t xpart = b.x - a.x, ypart = b.y - a.y;
  xpart *= xpart; ypart *= ypart;
  return sqrt(xpart + ypart);
}

void dbgTouch() { // print current state of touch model //////////////////
  D("p(x:%04d,y:%04d,z:%03d)", p.x, p.y, p.z)
  D(", now=%05lu, sincePrevSig=%05lu, prevSig=", now, sincePrevSig)
  D("p(x:%04d,y:%04d,z:%03d)", prevSig.x, prevSig.y, prevSig.z)
  D(", prevSigMillis=%05lu, fromPrevSig=%05u", prevSigMillis, fromPrevSig)
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Accept or reject touch signals
 */
bool UIController::gotTouch() { 
  if(!ts.touched()) {
    return false; // no touches
  }
    
  // set up timings
  now = millis();
  if(firstTimeThrough) {
    sincePrevSig = TIME_SENSITIVITY + 1;
  } else {
    sincePrevSig = now - prevSigMillis;
  }

  // retrieve a point
  p = ts.getPoint();
  // TODO should we read the rest of the buffer? 
  //  while (! touch.bufferEmpty()) {

  // if it is at 0,0,0 and we've just started then ignore it
  if(p == firstTouch && firstTimeThrough) {
    dbgTouch();
    D(", rejecting (0)\n\n")
    return false;
  }
  firstTimeThrough = false;
  
  // calculate distance from previous signal
  fromPrevSig = distanceBetween(p, prevSig);
  dbgTouch();

  D(", sincePrevSig<TIME_SENS.: %d...  ", sincePrevSig<TIME_SENSITIVITY)
  if(sincePrevSig < TIME_SENSITIVITY) { // ignore touches too recent
    D("rejecting (2)\n")
  } else if(
    fromPrevSig < DIST_SENSITIVITY && sincePrevSig < TREAT_AS_NEW
  ) {
    D("rejecting (3)\n")
  } else {
    prevSig = p;
    prevSigMillis = now;
    D("decided this is a new touch\n")
    return true;
  }
  return false;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void UIController::changeMode() {
  D("changing mode from %d to...", m_mode)
  tft.fillScreen(HX8357_BLACK);

  setTimeSensitivity(); // set TIME_SENS to the default

  switch(m_mode) {
    case ui_touchpaint:
      m_mode = ui_configure;  m_element = new ConfigUIElement(&tft, &ts);
      break;
    case ui_qwerty:    
      m_mode = ui_text;       m_element = new TextPageUIElement(&tft, &ts);
      break;
    case ui_configure: 
      m_mode = ui_mic;        m_element = new MicPlayerUIElement(&tft, &ts);
      break;
    case ui_mic:       
      m_mode = ui_music;      m_element = new MusicPlayerUIElement(&tft, &ts);
      break;
    case ui_music:     
      m_mode = ui_text;       m_element = new TextPageUIElement(&tft, &ts);
      break;
    case ui_text:      
      m_mode = ui_testcard;   m_element = new TestCardUIElement(&tft, &ts);
      break;
    case ui_testcard:  
      m_mode = ui_touchpaint; setTimeSensitivity(25); 
      m_element = new TouchpaintUIElement(&tft, &ts);
      break;
    default:           
      m_mode = ui_text;       m_element = new TextPageUIElement(&tft, &ts);
  }
  D("...%d\n", m_mode)

  m_element->draw();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void UIController::handleTouch() {
  p.x = map(p.x, TS_MAXX, TS_MINX, tft.width(), 0);
  p.y = map(p.y, TS_MAXY, TS_MINY, 0, tft.height());

  // previously, before screen rotation in unphone spin 4, we did it like
  // this (which is probably from the Adafruit example):
  // p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
  // p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  
  if (m_element->handleTouch(p.x, p.y)) {
    if (modeChangeRequests++ >= 2) {
      changeMode();
      modeChangeRequests = 0;
    }
  } 

}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void UIController::run() {
  if(gotTouch()) {
    handleTouch();
  } 
  m_element->runEachTurn();

}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * turn vibration motor on or off
 * @param on - type bool 
 */
void UIController::vibemotor(bool on) { 
  if(on)
    ts.writeRegister8(0x10, 4); // set GPIO2 HIGH
  else
    ts.writeRegister8(0x11, 4); // set GPIO2 LOW
}
//////////////////////////////////////////////////////////////////////////
// backlight enable & vibe disable
    
//////////////////////////////////////////////////////////////////////////
/**
 * turn the backlight on or off
 * @param on - type bool
 */
void UIController::backlight(bool on) { 
  if(on)
    digitalWrite(2, HIGH);
  else
    digitalWrite(2, LOW);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Defined getMAC function. Has to reverse it...
 * this might need moving 
 */
char *getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion..
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
  return buf;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void UIElement::drawSwitcher(uint16_t xOrigin, uint16_t yOrigin) {
  uint16_t leftX = xOrigin;
  if(leftX == 0)
    leftX = (SWITCHER * BOXSIZE) + 8; // default is on right hand side
  m_tft->fillRect(leftX, 15 + yOrigin, BOXSIZE - 15, HALFBOX - 10, WHITE);
  m_tft->fillTriangle(
    leftX + 15, 35 + yOrigin,
    leftX + 15,  5 + yOrigin,
    leftX + 30, 20 + yOrigin,
    WHITE
  );
}
//////////////////////////////////////////////////////////////////////////
