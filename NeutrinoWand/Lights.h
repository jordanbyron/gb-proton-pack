#ifndef Cyclotron_h
#define Cyclotron_h
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
  void vent(unsigned long currentMillis);
  void off(unsigned long currentMillis);
private:
  Adafruit_NeoPixel *_lights;
  int _pin;
  int _numberOfPixels;
};
#endif