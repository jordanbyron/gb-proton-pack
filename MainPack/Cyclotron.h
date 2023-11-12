#ifndef Cyclotron_h
#define Cyclotron_h
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
class Cyclotron {
public:
  Cyclotron(int16_t pin, uint16_t cyclotronStart, uint16_t countLedsPerCyclotron, uint16_t ventStart, uint16_t countVentLeds);
  void setup(void);
  void clear(void);
  void boot(unsigned long currentMillis);
  void idle(unsigned long currentMillis, unsigned long anispeed);
  void vent(unsigned long currentMillis);
  void off(unsigned long currentMillis);
private:
  Adafruit_NeoPixel *_lights;
  int _pin;
  int _numberOfLeds;
  uint16_t _cyclotronStart;
  uint16_t _countLedsPerCyclotron;
  uint16_t _ventStart;
  uint16_t _ventEnd;
  uint16_t _countVentLeds;
  void _setCyclotronLightState(int startLed, int endLed, int state);
};
#endif