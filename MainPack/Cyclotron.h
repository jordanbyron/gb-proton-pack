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
  int _c1Start;
  int _c1End;
  int _c2Start;
  int _c2End;
  int _c3Start;
  int _c3End;
  int _c4Start;
  int _c4End;
  unsigned long _prevCycBootMillis = 0;
  bool _reverseBootCyclotron = false;
  int _cycOrder = 0;
  unsigned long _prevCycMillis = 0;
  unsigned long _prevShtdMillis = 0;
  int _cyclotronFadeOut = 175;
  void _setCyclotronLightState(int startLed, int endLed, int state);
};
#endif