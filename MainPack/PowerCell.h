#ifndef PowerCell_h
#define PowerCell_h
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <FireTimer.h>
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
  unsigned long _prevPwrBootMillis = 0;
  unsigned long _prevPwrMillis = 0;
  int _powerSeqNum;
  int _powerShutdownSeqNum;
  int _currentBootLevel;
  int _currentLightLevel;
  bool _powerBoot = false;
  bool _animationComplete = false;
  FireTimer _offTimer;
};
#endif