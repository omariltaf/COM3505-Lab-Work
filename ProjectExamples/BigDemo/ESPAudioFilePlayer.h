// ESPAudioFilePlayer.h

#ifndef ESPMUSICPLAYER_H_
#define ESPMUSICPLAYER_H_

#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of ESPMusicPlayer class. 
 */
class ESPAudioFilePlayer {
  private:
    String m_file;
    uint8_t m_volume;
    Adafruit_VS1053_FilePlayer* m_musicPlayer;
  public:
    ESPAudioFilePlayer(String file, uint8_t volume);
    bool begin();
    bool isEnded();
    bool play();
};
//////////////////////////////////////////////////////////////////////////////

#endif
