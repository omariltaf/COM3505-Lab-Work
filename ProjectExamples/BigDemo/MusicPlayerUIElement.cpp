// MusicPlayerUIElement.cpp

#include "AllUIElement.h"

//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool MusicPlayerUIElement::handleTouch(long x, long y) {
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
void MusicPlayerUIElement::draw(){
  printf("\n\ngot to here0.0\n\n");

  m_tft->fillScreen(BLACK);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);

  m_tft->setCursor(0, 0);
  m_tft->print("[music playing mode]");
  drawSwitcher();

  m_tft->setCursor(0, 20);

  printf("\n\ngot to here0.1\n\n");

  if (m_musicPlayer->begin()) {
    printf("\n\ngot to here0.2\n\n");
    if (m_musicPlayer->play()) {
      m_tft->print("Playing music now");
    } else {
      m_tft->print("Playing music failed");
    }

  } else {
    m_tft->print("begin failed!");
  }
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/**
 * Funtion that runs each turn, once the music has finished playing it displays a message
 */
void MusicPlayerUIElement::runEachTurn(){
  // Do nothing 
  if(m_musicPlayer->isEnded() && m_once) {
    m_tft->setCursor(0,60);
    m_tft->print("MUSIC FINISHED!");
    m_once = false;
  }
}
//////////////////////////////////////////////////////////////////////////

ESPAudioFilePlayer::ESPAudioFilePlayer (String file, uint8_t volume) {
  m_file   = file;
  m_volume = volume;
  m_musicPlayer = &musicPlayer;
}

bool ESPAudioFilePlayer::begin(){
  if (m_musicPlayer->begin()) {
    //m_musicPlayer->sineTest(0x44, 500); // Make a tone to indicate VS1053 is working

    #if defined(__AVR_ATmega32U4__) 
      // Timer interrupts are not suggested, better to use DREQ interrupt!
      // but we don't have them on the 32u4 feather...
      printf("\ncock\n");
      m_musicPlayer->useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
    #elif defined(ESP32)
      // no IRQ! doesn't work yet :/
      printf("\nballs\n");
      pinMode(VS1053_DREQ, INPUT_PULLUP);
      m_musicPlayer->useInterrupt(VS1053_DREQ);  // DREQ int
    #else
      // If DREQ is on an interrupt pin we can do background
      // audio playing
      printf("\nWHICH\n");
      m_musicPlayer->useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
    #endif
     
    return true;
  }
  return false;
}

bool ESPAudioFilePlayer::play(){
  if (!SD.exists(m_file)) {
    printf("\nFile doesn't exist!!\n");
    return false;
  }

  printf("\nSetting volume\n");
  // Set volume for left, right channels. lower numbers == louder volume!
  m_musicPlayer->setVolume(m_volume,m_volume);

  printf("\nPlaying file\n");
  return m_musicPlayer->playFullFile(m_file.c_str());
}

bool ESPAudioFilePlayer::isEnded(){
  return m_musicPlayer->stopped();
}

