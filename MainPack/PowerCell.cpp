#include "Arduino.h"
#include "PowerCell.h"
#include <Adafruit_NeoPixel.h>
#include <FireTimer.h>

// timer helpers and intervals for the animations
const int powercellLedCount = 14;    // total number of led's in the animation
const int powercellIndexOffset = 0;  // first led offset into the led chain for the animation

const unsigned long pwr_boot_interval = 30;  // interval at which to cycle lights (milliseconds). Adjust this if

const int powerSeqTotal = powercellLedCount;                         // total number of led's for powercell 0 based

PowerCell::PowerCell(uint16_t numberOfLeds, int16_t pin) {
  _numberOfLeds = numberOfLeds;
  _pin = pin;
  _powerSeqNum = powercellIndexOffset;
  _powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
  _currentBootLevel = powercellIndexOffset;
  _currentLightLevel = powercellLedCount - powercellIndexOffset;
}

void PowerCell::setup() {
  this->_lights = new Adafruit_NeoPixel(this->_numberOfLeds, this->_pin, NEO_GRB + NEO_KHZ800);

  this->_lights->begin();
  this->_lights->setBrightness(75);
  this->_lights->show();  // Initialize all pixels to 'off'
}

void PowerCell::boot(unsigned long currentMillis) {
  bool doUpdate = false;

  if (this->_powerBoot == true) { 
    this->idle(currentMillis, 1000);
    return;
  }

  if ((unsigned long)(currentMillis - this->_prevPwrBootMillis) >= pwr_boot_interval) {
    // save the last time you blinked the LED
    this->_prevPwrBootMillis = currentMillis;

    // START POWERCELL
    if (this->_currentBootLevel != powerSeqTotal) {
      if (this->_currentBootLevel == this->_currentLightLevel) {
        if (this->_currentLightLevel + 1 <= powerSeqTotal) {
          this->_lights->setPixelColor(this->_currentLightLevel + 1, 0);
        }
        this->_lights->setPixelColor(this->_currentBootLevel, this->_lights->Color(0, 0, 255));
        this->_currentLightLevel = powerSeqTotal;
        this->_currentBootLevel++;
      } else {
        if (this->_currentLightLevel + 1 <= powerSeqTotal) {
          this->_lights->setPixelColor(this->_currentLightLevel + 1, 0);
        }
        this->_lights->setPixelColor(this->_currentLightLevel, this->_lights->Color(0, 0, 255));
        this->_currentLightLevel--;
      }
      doUpdate = true;
    } else {
      this->_powerBoot = true;
      this->_currentBootLevel = powercellIndexOffset;
      this->_currentLightLevel = powercellLedCount - powercellIndexOffset;
    }
    // END POWERCELL
  }

  if (doUpdate) {
    this->_lights->show();
  }
}

void PowerCell::idle(unsigned long currentMillis, unsigned long anispeed) {
  bool doUpdate = false;

  // START POWERCELL
  if ((unsigned long)(currentMillis - this->_prevPwrMillis) >= anispeed) {
    // save the last time you blinked the LED
    this->_prevPwrMillis = currentMillis;

    for (int i = powercellIndexOffset; i <= powerSeqTotal; i++) {
      if (i <= this->_powerSeqNum) {
        this->_lights->setPixelColor(i, this->_lights->Color(0, 0, 150));
      } else {
        this->_lights->setPixelColor(i, 0);
      }
    }

    if (this->_powerSeqNum <= powerSeqTotal) {
      this->_powerSeqNum++;
    } else {
      this->_powerSeqNum = powercellIndexOffset;
    }

    doUpdate = true;
  }
  // END POWERCELL

  // if we changed anything update
  if (doUpdate == true) {
    this->_lights->show();
  }
}

// FIXME: I think this is actually Overload
void PowerCell::off(bool start) {
  if(start) { 
    this->_animationComplete = false;
    this->_offTimer.begin(175);
  }

  if(this->_animationComplete) return;

  if (this->_offTimer.fire()) {
    for (int i = powerSeqTotal; i >= powercellIndexOffset; i--) {
      if (i <= this->_powerShutdownSeqNum) {
        this->_lights->setPixelColor(i, this->_lights->Color(0, 0, 150));
      } else {
        this->_lights->setPixelColor(i, 0);
      }
    }

    this->_lights->show();

    if (this->_powerShutdownSeqNum >= powercellIndexOffset) {
      this->_powerShutdownSeqNum--;
    } else {
      this->_powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
      this->_animationComplete = true;
    }
  }
}

void PowerCell::clear() {
  this->_powerSeqNum = powercellIndexOffset;
  this->_powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
  this->_currentLightLevel = powercellLedCount;
  this->_currentBootLevel = powercellIndexOffset;
  this->_powerBoot = false;
  this->_animationComplete = false;

  this->_lights->clear();
  this->_lights->show();
}