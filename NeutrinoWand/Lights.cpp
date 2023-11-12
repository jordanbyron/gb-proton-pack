#include "Arduino.h"
#include "Lights.h"
#include <Adafruit_NeoPixel.h>
#include <FireTimer.h>

// These are the indexes for the led's on the chain.
const int frontHatLight = 0;
const int ventLight = 1;
const int topArcoelectricLight = 2;
const int topHatLight = 3;
const int slowbloLight = 4;

Lights::Lights(int16_t pin) {
  this->_pin = pin;
  this->_numberOfPixels = 5;
}

void Lights::setup() {
  this->_lights = new Adafruit_NeoPixel(this->_numberOfPixels, this->_pin, NEO_GRB + NEO_KHZ800);

  this->_lights->begin();
  this->_lights->setBrightness(100);
  this->_lights->show();  // Initialize all pixels to 'off'
}

const unsigned long bootBlinkInterval = 750;  // interval at which to cycle lights (milliseconds).
bool bootBlink = false;

FireTimer blinkTimer;

void Lights::boot(bool init) {
  if (init) {
    blinkTimer.begin(bootBlinkInterval);
    bootBlink = false;
    this->clear(false);
  }

  if (blinkTimer.fire() || init) {
    bootBlink = !bootBlink;

    if (bootBlink) {
      this->_lights->setPixelColor(slowbloLight, this->_lights->Color(255, 0, 0));
    } else {
      this->_lights->setPixelColor(slowbloLight, this->_lights->Color(0, 0, 0));
    }

    this->_lights->show();
  }
}

void Lights::locked(bool init) {
  if (init) {
    this->clear(false);
    this->_lights->setPixelColor(slowbloLight, this->_lights->Color(255, 0, 0));  // Sloblo on steady
    this->_lights->show();
  }
}

void Lights::activated(bool init) {
  if (init) {
    this->clear(false);
    this->_lights->setPixelColor(slowbloLight, this->_lights->Color(255, 0, 0));   // Sloblo on steady
    this->_lights->setPixelColor(ventLight, this->_lights->Color(255, 255, 255));  // Vent lights on steady
    this->_lights->show();

    blinkTimer.begin(bootBlinkInterval);
    bootBlink = false;
  }

  if (blinkTimer.fire() || init) {
    bootBlink = !bootBlink;

    if (bootBlink) {
      this->_lights->setPixelColor(frontHatLight, this->_lights->Color(255, 255, 255));
      this->_lights->setPixelColor(topHatLight, this->_lights->Color(0, 0, 0));
    } else {
      this->_lights->setPixelColor(frontHatLight, this->_lights->Color(0, 0, 0));
      this->_lights->setPixelColor(topHatLight, this->_lights->Color(255, 255, 255));
    }

    this->_lights->show();
  }
}

int overloadInterval = 200;
FireTimer arcoelectricBlinkTimer;
bool arcoelectricBlink = false;

void Lights::overload(bool init) {
  if (init) {
    this->clear(false);
    this->_lights->setPixelColor(ventLight, this->_lights->Color(255, 255, 255));  // Vent lights on steady
    this->_lights->show();

    blinkTimer.begin(overloadInterval);
    arcoelectricBlinkTimer.begin(100);
    bootBlink = false;
    arcoelectricBlink = false;
  }

  if (arcoelectricBlinkTimer.fire()) {
    arcoelectricBlink = !arcoelectricBlink;

    if (arcoelectricBlink) {
      this->_lights->setPixelColor(topArcoelectricLight, this->_lights->Color(255, 255, 255));
    } else {
      this->_lights->setPixelColor(topArcoelectricLight, this->_lights->Color(0, 0, 0));
    }

    this->_lights->show();
  }

  if (blinkTimer.fire() || init) {
    bootBlink = !bootBlink;

    if (bootBlink) {
      this->_lights->setPixelColor(frontHatLight, this->_lights->Color(255, 255, 255));
      this->_lights->setPixelColor(topHatLight, this->_lights->Color(0, 0, 0));
      this->_lights->setPixelColor(slowbloLight, this->_lights->Color(255, 0, 0));
    } else {
      this->_lights->setPixelColor(frontHatLight, this->_lights->Color(0, 0, 0));
      this->_lights->setPixelColor(topHatLight, this->_lights->Color(255, 255, 255));
      this->_lights->setPixelColor(slowbloLight, this->_lights->Color(0, 0, 0));
    }

    this->_lights->show();
  }
}

void Lights::clear(bool update = true) {
  this->_lights->clear();
  this->_lights->setBrightness(100);
  if (update) { this->_lights->show(); }
}