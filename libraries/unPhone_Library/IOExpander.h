// IOExpander.h 
// Jon Williamson, Pimoroni, Oct/Nov 2018

#include <stdint.h>
#ifndef IO_EXPANDER_H
#define IO_EXPANDER_H

/*
The unPhone has a TCA9555 IO expansion chip that is controlled over I2C and to
which the SPI chip select (CS), reset and etc. lines of many of the modules
are connected. To use these modules the IO expander has to be told to trigger
those lines. This means that the available libraries for the modules also need
to be adapted to talk to the IOExpander, e.g. when doing digitalWrite or
pinMode. We do this by injecting code to call our own versions of these
functions (defined below) and setting the second highest bit of the pin number
high to signal those pins that are controlled via the TCA9555. Confused? Not
nearly as much as I have been...

Usage notes:

call `IOExpander::begin()` in `setup()` method after `Wire.begin()`
then to interface with the IO Expander pins you can do:
`IOExpander::digitalWrite(IOExpander::SD_CS, LOW)`

to read the current board revision use:
`uint8_t version = IOExpander::getVersionNumber()`

to get the power switch position use:
`uint8_t switch_state = IOExpander::digitalRead(IOExpander::POWER_SWITCH);`
*/
class IOExpander {
  public:
    static const uint8_t VERSION_1     =  0;
    static const uint8_t VERSION_2     = 12;
    static const uint8_t VERSION_3     = 15;
    
    static const uint8_t LCD_RESET     =  1 | 0x40;
    static const uint8_t BACKLIGHT     =  2 | 0x40;
    static const uint8_t LCD_CS        =  3 | 0x40;
    static const uint8_t LORA_CS       =  4 | 0x40;
    static const uint8_t LORA_RESET    =  5 | 0x40;
    static const uint8_t TOUCH_CS      =  6 | 0x40;
    static const uint8_t MUSIC_CS      =  7 | 0x40;
    static const uint8_t MUSIC_DCS     =  8 | 0x40;
    static const uint8_t MUSIC_RESET   =  9 | 0x40;
    static const uint8_t POWER_SWITCH  = 10 | 0x40;
    static const uint8_t SD_CS         = 11 | 0x40;
    static const uint8_t VIBRATION     = 13 | 0x40;
    static const uint8_t USB_VSENSE    = 14 | 0x40;

    // A0 = GND, A1 = 3V3, A2 = 3V3
    static const uint8_t i2c_address = 0x26;

    // we cache the current state of the ports after
    // an initial read of the values during initialisation
    static uint16_t directions;
    static uint16_t output_states;

    static void begin();
    static void pinMode(uint8_t pin, uint8_t mode);
    static void digitalWrite(uint8_t pin, uint8_t value);
    static uint8_t digitalRead(uint8_t pin);
    static uint8_t getVersionNumber();

  private:
    static uint16_t readRegisterWord(uint8_t reg);
    static void writeRegisterWord(uint8_t reg, uint16_t value);
};

#endif
