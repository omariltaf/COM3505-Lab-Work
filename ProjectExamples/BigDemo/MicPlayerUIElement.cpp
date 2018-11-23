// MicPlayerUIElement.cpp

#include "AllUIElement.h"

#include "driver/i2s.h"

//////////////////////////////////////////////////////////////////////////
/**
 * Function to configure the microphone settings
 */
void MicPlayerUIElement::i2s_config() {
  // http://esp-idf.readthedocs.io/en/latest/api/peripherals/i2s.html
  // input
  i2s_config_t i2s_in_config = {
    mode: (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    sample_rate: 44100,
    bits_per_sample: (i2s_bits_per_sample_t)32,
    channel_format: I2S_CHANNEL_FMT_RIGHT_LEFT,
    communication_format: (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    intr_alloc_flags: ESP_INTR_FLAG_LEVEL1,
    dma_buf_count: 14,
    dma_buf_len: 64
  };
  i2s_pin_config_t i2s_in_pin_config = {
    bck_io_num: 16,
    ws_io_num: 4,
    data_out_num: -1, //Not used
    data_in_num: 35
  };

   pinMode(35, INPUT);  // aka pin TX connected to Data OUT (DOUT)
   pinMode(16, OUTPUT); // aka pin RX connected to Bit Clock (BCLK) aka Clock
   pinMode(4, OUTPUT); // aka pin A5 connected to Left Right Clock (LRCL) also known as Word Select (WS)

  i2s_driver_install((i2s_port_t)0, &i2s_in_config, 0, NULL);
  i2s_set_pin((i2s_port_t)0, &i2s_in_pin_config);
}

/**
 * Function to read the value recorded by the microphone
 */
void MicPlayerUIElement::i2s_read() {
  uint32_t sample_val[2] = {0, 0};
  uint8_t bytes_read = i2s_pop_sample((i2s_port_t)0, (char *)sample_val, portMAX_DELAY);
  printf ("Read: %d \n", sample_val[0] << 5);
}


//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool MicPlayerUIElement::handleTouch(long x, long y) {
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
void MicPlayerUIElement::draw(){

  m_timer = millis();

  m_tft->fillScreen(BLACK);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);

  m_tft->setCursor(0, 0);
  m_tft->print("[mic listening mode]");
  drawSwitcher();

  i2s_config();
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/**
 * Funtion that runs each turn, once the music has finished playing it displays a message
 */
void MicPlayerUIElement::runEachTurn(){
  if(millis() - m_timer < 10000) {
    i2s_read();
  } else {
    m_tft->setCursor(0, 20);
    m_tft->print("Mic finished listening!");
  }
}
//////////////////////////////////////////////////////////////////////////
