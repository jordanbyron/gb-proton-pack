#include "Arduino.h"
#include "Cyclotron.h"
#include <Adafruit_NeoPixel.h>

// These are the indexes for the led's on the chain.
int c1Start;
int c1End;
int c2Start;
int c2End;
int c3Start;
int c3End;
int c4Start;
int c4End;

Cyclotron::Cyclotron(int16_t pin, uint16_t cyclotronStart, uint16_t countLedsPerCyclotron, uint16_t ventStart, uint16_t countVentLeds) {
  this->_pin = pin;
  this->_cyclotronStart = cyclotronStart;
  this->_countLedsPerCyclotron = countLedsPerCyclotron;
  this->_ventStart = ventStart;
  this->_ventEnd = ventStart + countVentLeds - 1;
  this->_numberOfLeds = this->_ventEnd + 1;

  int cyclotronLedOffset = this->_countLedsPerCyclotron - 1;

  c1Start = this->_cyclotronStart;
  c1End = c1Start + cyclotronLedOffset;
  c2Start = c1End + 1;
  c2End = c2Start + cyclotronLedOffset;
  c3Start = c2End + 1;
  c3End = c3Start + cyclotronLedOffset;
  c4Start = c3End + 1;
  c4End = c4Start + cyclotronLedOffset;
}

void Cyclotron::setup() {
  this->_lights = new Adafruit_NeoPixel(this->_numberOfLeds, this->_pin, NEO_GRB + NEO_KHZ800);

  this->_lights->begin();
  this->_lights->setBrightness(75);
  this->_lights->show();  // Initialize all pixels to 'off'
}

unsigned long prevCycBootMillis = 0;
const unsigned long cyc_boot_interval = 500;  // interval at which to cycle lights (milliseconds).
bool reverseBootCyclotron = false;

void Cyclotron::boot(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevCycBootMillis) >= cyc_boot_interval) {
    prevCycBootMillis = currentMillis;

    if (reverseBootCyclotron == false) {
      _setCyclotronLightState(c1Start, c1End, 1);
      _setCyclotronLightState(c2Start, c2End, 2);
      _setCyclotronLightState(c3Start, c3End, 1);
      _setCyclotronLightState(c4Start, c4End, 2);

      reverseBootCyclotron = true;
    } else {
      _setCyclotronLightState(c1Start, c1End, 2);
      _setCyclotronLightState(c2Start, c2End, 1);
      _setCyclotronLightState(c3Start, c3End, 2);
      _setCyclotronLightState(c4Start, c4End, 1);

      reverseBootCyclotron = false;
    }

    this->_lights->show();
  }
}

int cycOrder = 0;                   // which cyclotron led will be lit next
unsigned long cyc_interval = 1000;  // interval at which to cycle lights for the cyclotron.
unsigned long prevCycMillis = 0;    // last time we changed a cyclotron light in the idle sequence

void Cyclotron::idle(unsigned long currentMillis, unsigned long cycspeed) {
  if ((unsigned long)(currentMillis - prevCycMillis) >= cycspeed) {
    prevCycMillis = currentMillis;

    switch (cycOrder) {
      case 0:
        _setCyclotronLightState(c4Start, c4End, 2);
        _setCyclotronLightState(c1Start, c1End, 0);
        _setCyclotronLightState(c2Start, c2End, 2);
        _setCyclotronLightState(c3Start, c3End, 2);
        cycOrder = 1;
        break;
      case 1:
        _setCyclotronLightState(c1Start, c1End, 2);
        _setCyclotronLightState(c2Start, c2End, 0);
        _setCyclotronLightState(c3Start, c3End, 2);
        _setCyclotronLightState(c4Start, c4End, 2);
        cycOrder = 2;
        break;
      case 2:
        _setCyclotronLightState(c1Start, c1End, 2);
        _setCyclotronLightState(c2Start, c2End, 2);
        _setCyclotronLightState(c3Start, c3End, 0);
        _setCyclotronLightState(c4Start, c4End, 2);
        cycOrder = 3;
        break;
      case 3:
        _setCyclotronLightState(c1Start, c1End, 2);
        _setCyclotronLightState(c2Start, c2End, 2);
        _setCyclotronLightState(c3Start, c3End, 2);
        _setCyclotronLightState(c4Start, c4End, 0);
        cycOrder = 0;
        break;
    }

    this->_lights->show();
  }
}

unsigned long prevShtdMillis = 0;                 // last time we changed a light in the idle sequence
const unsigned long pwr_shutdown_interval = 50;  // interval at which to cycle lights (milliseconds).
int cyclotronFadeOut = 175;

void Cyclotron::off(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevShtdMillis) >= pwr_shutdown_interval) {
    prevShtdMillis = currentMillis;

    for (int i = c1Start; i <= c4End; i++) {
      if (cyclotronFadeOut >= 0) {
        this->_lights->setPixelColor(i, 255 * cyclotronFadeOut / 255, 0, 0);
        cyclotronFadeOut--;
      } else {
        this->_lights->setPixelColor(i, 0);
      }
    }

    this->_lights->show();
  }
}

void Cyclotron::vent(unsigned long currentMillis) {
  for (int i = this->_ventStart; i <= this->_ventEnd; i++) {
    this->_lights->setPixelColor(i, this->_lights->Color(255, 255, 255));
  }
  this->_lights->setBrightness(100);
  this->_lights->show();
}

void Cyclotron::clear() {
  this->_lights->clear();
  this->_lights->show();
  this->_lights->setBrightness(75);
  prevShtdMillis = 0;
  cyclotronFadeOut = 175;
}

void Cyclotron::_setCyclotronLightState(int startLed, int endLed, int state) {
  switch (state) {
    case 0:  // set all leds to red
      for (int i = startLed; i <= endLed; i++) {
        this->_lights->setPixelColor(i, this->_lights->Color(255, 0, 0));
      }
      break;
    case 1:  // set all leds to orange
      for (int i = startLed; i <= endLed; i++) {
        this->_lights->setPixelColor(i, this->_lights->Color(255, 106, 0));
      }
      break;
    case 2:  // set all leds off
      for (int i = startLed; i <= endLed; i++) {
        this->_lights->setPixelColor(i, 0);
      }
      break;
  }
}