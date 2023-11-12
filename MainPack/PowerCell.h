#ifndef PowerCell_h
#define PowerCell_h
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
class PowerCell {
public:
  // Constructor: number of LEDs, pin number, LED type
  PowerCell(uint16_t numberOfLeds, int16_t pin);
  void setup(void);
  void clear(void);
  void boot(unsigned long currentMillis);
  void idle(unsigned long currentMillis, unsigned long anispeed);
  void off(bool);
private:
  Adafruit_NeoPixel *_lights;
  int _pin;
  int _numberOfLeds;
};
#endif