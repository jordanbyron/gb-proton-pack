#ifndef Lights_h
#define Lights_h
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
class Lights {
public:
  Lights(int16_t pin);
  void setup(void);
  void clear(bool update = true);
  void boot(bool init);
  void locked(bool init);
  void activated(bool init);
  void overload(bool init);
private:
  Adafruit_NeoPixel *_lights;
  int _pin;
  int _numberOfPixels;
};
#endif