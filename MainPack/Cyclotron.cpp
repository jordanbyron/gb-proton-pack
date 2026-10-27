#include "Arduino.h"
#include "Cyclotron.h"
#include <Adafruit_NeoPixel.h>

Cyclotron::Cyclotron(int16_t pin, uint16_t cyclotronStart, uint16_t countLedsPerCyclotron, uint16_t ventStart, uint16_t countVentLeds) {
  this->_pin = pin;
  this->_cyclotronStart = cyclotronStart;
  this->_countLedsPerCyclotron = countLedsPerCyclotron;
  this->_ventStart = ventStart;
  this->_ventEnd = ventStart + countVentLeds - 1;
  this->_numberOfLeds = this->_ventEnd + 1;

  int cyclotronLedOffset = this->_countLedsPerCyclotron - 1;

  this->_c1Start = this->_cyclotronStart;
  this->_c1End = this->_c1Start + cyclotronLedOffset;
  this->_c2Start = this->_c1End + 1;
  this->_c2End = this->_c2Start + cyclotronLedOffset;
  this->_c3Start = this->_c2End + 1;
  this->_c3End = this->_c3Start + cyclotronLedOffset;
  this->_c4Start = this->_c3End + 1;
  this->_c4End = this->_c4Start + cyclotronLedOffset;
}

void Cyclotron::setup() {
  this->_lights = new Adafruit_NeoPixel(this->_numberOfLeds, this->_pin, NEO_GRB + NEO_KHZ800);

  this->_lights->begin();
  this->_lights->setBrightness(100);
  this->_lights->show();  // Initialize all pixels to 'off'
}

const unsigned long cyc_boot_interval = 500;  // interval at which to cycle lights (milliseconds).

void Cyclotron::boot(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - this->_prevCycBootMillis) >= cyc_boot_interval) {
    this->_prevCycBootMillis = currentMillis;

    if (this->_reverseBootCyclotron == false) {
      _setCyclotronLightState(this->_c1Start, this->_c1End, 1);
      _setCyclotronLightState(this->_c2Start, this->_c2End, 2);
      _setCyclotronLightState(this->_c3Start, this->_c3End, 1);
      _setCyclotronLightState(this->_c4Start, this->_c4End, 2);

      this->_reverseBootCyclotron = true;
    } else {
      _setCyclotronLightState(this->_c1Start, this->_c1End, 2);
      _setCyclotronLightState(this->_c2Start, this->_c2End, 1);
      _setCyclotronLightState(this->_c3Start, this->_c3End, 2);
      _setCyclotronLightState(this->_c4Start, this->_c4End, 1);

      this->_reverseBootCyclotron = false;
    }

    this->_lights->show();
  }
}

void Cyclotron::idle(unsigned long currentMillis, unsigned long cycspeed) {
  if ((unsigned long)(currentMillis - this->_prevCycMillis) >= cycspeed) {
    this->_prevCycMillis = currentMillis;

    switch (this->_cycOrder) {
      case 0:
        _setCyclotronLightState(this->_c4Start, this->_c4End, 2);
        _setCyclotronLightState(this->_c1Start, this->_c1End, 0);
        _setCyclotronLightState(this->_c2Start, this->_c2End, 2);
        _setCyclotronLightState(this->_c3Start, this->_c3End, 2);
        this->_cycOrder = 1;
        break;
      case 1:
        _setCyclotronLightState(this->_c1Start, this->_c1End, 2);
        _setCyclotronLightState(this->_c2Start, this->_c2End, 0);
        _setCyclotronLightState(this->_c3Start, this->_c3End, 2);
        _setCyclotronLightState(this->_c4Start, this->_c4End, 2);
        this->_cycOrder = 2;
        break;
      case 2:
        _setCyclotronLightState(this->_c1Start, this->_c1End, 2);
        _setCyclotronLightState(this->_c2Start, this->_c2End, 2);
        _setCyclotronLightState(this->_c3Start, this->_c3End, 0);
        _setCyclotronLightState(this->_c4Start, this->_c4End, 2);
        this->_cycOrder = 3;
        break;
      case 3:
        _setCyclotronLightState(this->_c1Start, this->_c1End, 2);
        _setCyclotronLightState(this->_c2Start, this->_c2End, 2);
        _setCyclotronLightState(this->_c3Start, this->_c3End, 2);
        _setCyclotronLightState(this->_c4Start, this->_c4End, 0);
        this->_cycOrder = 0;
        break;
    }

    this->_lights->show();
  }
}

const unsigned long pwr_shutdown_interval = 50;  // interval at which to cycle lights (milliseconds).

void Cyclotron::off(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - this->_prevShtdMillis) >= pwr_shutdown_interval) {
    this->_prevShtdMillis = currentMillis;

    for (int i = this->_c1Start; i <= this->_c4End; i++) {
      if (this->_cyclotronFadeOut >= 0) {
        this->_lights->setPixelColor(i, 255 * this->_cyclotronFadeOut / 255, 0, 0);
        this->_cyclotronFadeOut--;
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
  this->_lights->setBrightness(100);
  this->_prevShtdMillis = 0;
  this->_cyclotronFadeOut = 175;
}

void Cyclotron::_setCyclotronLightState(int startLed, int endLed, int state) {
  switch (state) {
    case 0:  // set all leds to red
      for (int i = startLed; i <= endLed; i++) {
        this->_lights->setPixelColor(i, this->_lights->Color(255, 255, 255));
      }
      break;
    case 1:  // set all leds to orange
      for (int i = startLed; i <= endLed; i++) {
        this->_lights->setPixelColor(i, this->_lights->Color(255, 165, 0));
      }
      break;
    case 2:  // set all leds off
      for (int i = startLed; i <= endLed; i++) {
        this->_lights->setPixelColor(i, 0);
      }
      break;
  }
}