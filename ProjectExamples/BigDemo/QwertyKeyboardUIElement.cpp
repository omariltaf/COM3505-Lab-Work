// TextPageUIElement.cpp

#include "AllUIElement.h"
#include <HTTPClient.h>

//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool QwertyKeyboardUIElement::handleTouch(long x, long y) {
    if(y < BOXSIZE && x > (BOXSIZE * SWITCHER)){
    return true;
  }

  m_tft->fillCircle(x, y, 10, WHITE);

  return false;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
// void QwertyKeyboardUIElement::printLetter(uint16_t x, uint16_t y) { 
  
// }
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
uint16_t dC = 0; // TODO
uint8_t QwertyKeyboardUIElement::mapKeyboardTouch(long xInput, long yInput) { //////////////////////////////////////
  int8_t sym = -1; // symbol
  int8_t row = 0;  // rows (0 is above the text entry area)
  int8_t col = 1;  // columns

  for(int y = 160, i = 1; y < 480; y += 80, i++)
    if(yInput > y) row = i;
  for(int x = 107, i = 2; x < 480; x += 107, i++)
    if(xInput > x) col = i;
  if(row > 0 && col >= 1)
    sym = (( (row - 1) * 3) + col) - 1;

  // D("row=%d, col=%d, sym=%d\n", row, col, sym)

  // TODO
  m_tft->setTextColor(WHITE, BLACK);
  char s[5];
  sprintf(s, "%2d,", sym);
  m_tft->setCursor(dC,150);
  dC += 3;
  if(dC > 280) dC = 0;
  m_tft->print(s);
  return sym;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void QwertyKeyboardUIElement::draw(){
  drawOutline();
  drawSwitcher();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Divide up the screen into areas and creates the keyboard
 */
void QwertyKeyboardUIElement::drawOutline() {

  // m_tft->setTextSize(2);
  // m_tft->setTextColor(BLUE);
  // const uint8_t NUMLABELS = 3;
  // const char *labels[NUMLABELS] = {
  //   " q  w e  r t  y  u  i  o p ",
  //   "UP a s d f g h j k l <-",
  //   "____ z x c v b n m ____"
  // };

  // int yLvl = 325;
  // for(int j = 0; j < 3; j++) {
  //   m_tft->setCursor(0, yLvl);
  //   m_tft->print(labels[j]);
  //   yLvl += 50;
  // }

  // const char *labels[NUMLABELS] = {
  //   " q"," w"," e"," r"," r"," t"," y"," u"," i"," o"," p",
  //   "UP"," a"," s"," d"," f"," g"," h"," j"," k"," l","<-",
  //   "__","__"," z"," x"," c"," v"," b"," n"," m","__","__",
  // };

  // set cursor to y = 15 x = 145
  // += 48 +=50
  // m_tft->setTextSize(2);
  // m_tft->setTextColor(BLUE);
  // for (int i = 145; i > 10; i -= 50)
  // for(int j = 0; j < 3; j++) {
  //   m_tft->setCursor(0, yLvl);
  //   m_tft->print(labels[j]);
  //   yLvl += 50;
  // }

  for(int y = 0; y < 480; y += 48) {
    m_tft->drawFastHLine(10, y, 150, MAGENTA);
  }
  for(int x = 10; x < 161; x +=50) {
    m_tft->drawFastVLine(x, 0, 480, MAGENTA);
  }
  
  // for(int i = 0, x = 30, y = 190; i < NUMLABELS; i++) {
  //   for(int j = 0; j < 3; j++, i++) {
  //     m_tft->setCursor(x, y);
  //     if(i == 0 || i == 9 || i == 10) m_tft->setTextColor(WHITE);
  //     m_tft->print(labels[i]);
  //     if(i == 0 || i == 9 || i == 10) m_tft->setTextColor(BLUE);
  //     x += 107;
  //     if(x > 344) x = 30;
  //   }
  //   i--;
  //   y += 80;
  //   if(y > 430) y = 190;
  // }
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Text page UI, run each turn
 */
void QwertyKeyboardUIElement::runEachTurn(){
  // Do nothing
}
//////////////////////////////////////////////////////////////////////////
