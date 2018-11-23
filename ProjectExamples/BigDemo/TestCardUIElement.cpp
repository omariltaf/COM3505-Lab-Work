// TestCardUIElement.cpp

#include "AllUIElement.h"

//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool TestCardUIElement::handleTouch(long x, long y) {
  return y < BOXSIZE && x > (BOXSIZE * SWITCHER);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Function that controls the drawing on the test page
 */
void TestCardUIElement::draw(){
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);

  drawBBC();
  drawTestcard();

  WAIT_A_SEC;

  drawSwitcher();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Test page function that runs each turn 
 */
void TestCardUIElement::runEachTurn(){
  // do nothing 
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * draws the bbc test image
 */
void TestCardUIElement::drawBBC() { 
  bmpDraw("/testcard.bmp", 0, 0);
  m_tft->setTextSize(2);
  m_tft->setTextColor(WHITE);
  m_tft->setCursor(20, 360); m_tft->print("please wait"); WAIT_MS(100)
  m_tft->setCursor(20, 340); m_tft->print("winding up elastic band:");
  WAIT_MS(600)
  for(int i = 0; i<8; i++) {
    m_tft->setCursor(150 + (i * 5), 360); m_tft->print(".");
    WAIT_MS(300)
  }
  WAIT_MS(300) WAIT_MS(300)
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * draw stuff to make screen dimensions obvious
 */
void TestCardUIElement::drawTestcard() {
  m_tft->fillScreen(BLACK);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(1);
  m_tft->fillCircle(160, 240 + 100, 1, RED);
  m_tft->setCursor(160 + 8, 340);
  m_tft->setTextColor(RED);
  m_tft->print("X:160");

  m_tft->fillCircle(160 + 100, 240, 1, CYAN);
  m_tft->setCursor(260 + 8, 240);
  m_tft->setTextColor(CYAN);
  m_tft->print("Y:240");

  // (we're 320 wide and 480 tall in portrait mode)
  //            X    Y    W    H
  m_tft->drawRect(  0,   0,  60,  60, GREEN);
  m_tft->drawRect(130, 215,  60,  60, GREEN);
  m_tft->drawRect(260, 420,  60,  60, GREEN);
  m_tft->fillTriangle(5, 443, 10, 433, 15, 443, MAGENTA);
  m_tft->fillRoundRect(153, 435, 20, 10, 4, RED);
  m_tft->fillRect(295, 435, 10, 10, MAGENTA);

  // TODO the below causes a hang (bad radius?); use to set up task WDT
  // registration of this task with recovery?
  // m_tft->fillRoundRect(150, 430, 20, 10, 10, RED);

  // label the green boxes
  m_tft->setTextColor(BLUE);
  m_tft->setCursor(3, 50); m_tft->print("60x60:0,0");
  m_tft->setCursor(130, 277); m_tft->print("60x60:130,215");
  m_tft->setCursor(241, 410); m_tft->print("60x60:260,420");

  // significant positions, text
  m_tft->setTextSize(3);
  m_tft->setTextColor(YELLOW);
  m_tft->setCursor(135,  10); m_tft->print("the");
  m_tft->setCursor(135,  45); m_tft->print("test");
  m_tft->setCursor(135,  80); m_tft->print("card");
  m_tft->setTextSize(2);
  m_tft->setTextColor(WHITE);
  m_tft->setCursor(  0,   0);   m_tft->print("0,0");
  m_tft->setCursor(  0, 300);   m_tft->print("0,300");
  // m_tft->setCursor(260,   0); m_tft->print("260,0"); (obscured by switcher box)
  m_tft->setCursor(248,  60); m_tft->print("248,60");
  m_tft->setCursor(100, 300); m_tft->print("100,300");
  m_tft->setCursor(150, 240); m_tft->print("150,240");
  m_tft->setCursor(200, 300); m_tft->print("200,300");
  m_tft->setCursor(235, 465); m_tft->print("235,465");
  m_tft->setCursor(  0, 465);   m_tft->print("0,465");

  m_tft->drawFastHLine(150, 160, 120, MAGENTA);
  m_tft->drawFastVLine(270, 160,  50, MAGENTA);
  m_tft->drawLine(150, 160, 270, 210, MAGENTA);
  m_tft->fillCircle(230, 180, 5, CYAN);

  // horizontal and vertical counters
  m_tft->setTextColor(GREEN);
  for(int i=0, j=1; i<=460; i+=20, j++) {
    if(j == 7) continue;
    m_tft->setCursor(70, i);
    if(j < 10) m_tft->print(" ");
    if(i == 460) m_tft->setTextColor(GREEN);
    m_tft->print(j);
    WAIT_MS(50)
    if(i == 460) {
      m_tft->setCursor(98, 467);
      m_tft->setTextSize(1);
      m_tft->print("(X:70, Y:");
      m_tft->print(i);
      m_tft->print(")");
      m_tft->setTextSize(2);
    }
  }
  m_tft->setCursor(0, 120);
  m_tft->setTextColor(GREEN);
  m_tft->print("01234567890123456789012345");
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/**
* This function opens a Windows Bitmap (BMP) file and
* displays it at the given coordinates.  It's sped up
* by reading many pixels worth of data at a time
* (rather than pixel by pixel).  Increasing the buffer
* size takes more of the Arduino's precious RAM but
* makes loading a little faster.  20 pixels seems a
* good balance.
*
* NOTE that this interacts badly with the GFX primitives; it seems that
* both libraries set a screen coordinate that doesn't get initialised to x/y
* (or 0/0 for that matter) when this runs....
*/
#define BUFFPIXEL 20
void TestCardUIElement::bmpDraw(const char *filename, uint8_t x, uint16_t y) {
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= m_tft->width()) || (y >= m_tft->height())) return;

  D("Loading image '%s'...\n", filename);

  // Open requested file on SD card
  IOExpander::digitalWrite(IOExpander::SD_CS, LOW); // TODO shouldn't need
  if (! SD.exists(filename) ) {
    D("File not found\n");
    return;
  } else {
    bmpFile = SD.open(filename);
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    D("File size: %u\n", read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    D("Image Offset: %u\n", bmpImageoffset);
    // Read DIB header
    D("Header size: %u\n", read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      D("Bit Depth: %u\n", bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        D(
          "Image size: w(%u)h(%u), tft w(%u),h(%u)\n",
          bmpWidth, bmpHeight, m_tft->width(), m_tft->height()
        )

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= m_tft->width())  w = m_tft->width()  - x;
        if((y+h-1) >= m_tft->height()) h = m_tft->height() - y;

        // Set TFT address window to clipped image bounds
        m_tft->setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            m_tft->pushColor(m_tft->color565(r,g,b));
          } // end pixel
        } // end scanline
        D("Loaded in %lu mS\n", millis() - startTime);
      } // end goodBmp
    }
  }

  bmpFile.close();
  IOExpander::digitalWrite(IOExpander::SD_CS, HIGH); // TODO shouldn't need
  if(!goodBmp) D("BMP format not recognized\n");
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/** These read 16- and 32-bit types from the SD card file.
* BMP data is stored little-endian, Arduino is little-endian too.
* May need to reverse subscript order if porting elsewhere.
*/
uint16_t TestCardUIElement::read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t TestCardUIElement::read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
/////////////////////////////////////////////////////////////////////////
