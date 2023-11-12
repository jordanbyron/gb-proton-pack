#include "Arduino.h"
#include "PowerCell.h"
#include <Adafruit_NeoPixel.h>
#include <FireTimer.h>

// timer helpers and intervals for the animations
const int powercellLedCount = 14;    // total number of led's in the animation
const int powercellIndexOffset = 0;  // first led offset into the led chain for the animation

unsigned long prevPwrBootMillis = 0;         // the last time we changed a powercell light in the boot sequence
const unsigned long pwr_boot_interval = 30;  // interval at which to cycle lights (milliseconds). Adjust this if

// LED tracking variables
const int powerSeqTotal = powercellLedCount;                         // total number of led's for powercell 0 based
int powerSeqNum = powercellIndexOffset;                              // current running powercell sequence led
int powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;  // shutdown sequence counts down

// animation level trackers for the boot and shutdown
int currentBootLevel = powercellIndexOffset;                       // current powercell boot level sequence led
int currentLightLevel = powercellLedCount - powercellIndexOffset;  // current powercell boot light sequence led

bool powerBoot = false;

FireTimer offTimer;

PowerCell::PowerCell(uint16_t numberOfLeds, int16_t pin) {
  _numberOfLeds = numberOfLeds;
  _pin = pin;
}

void PowerCell::setup() {
  this->_lights = new Adafruit_NeoPixel(this->_numberOfLeds, this->_pin, NEO_GRB + NEO_KHZ800);

  this->_lights->begin();
  this->_lights->setBrightness(75);
  this->_lights->show();  // Initialize all pixels to 'off'
}

void PowerCell::boot(unsigned long currentMillis) {
  bool doUpdate = false;

  if (powerBoot == true) { 
    this->idle(currentMillis, 1000);
    return;
  }

  if ((unsigned long)(currentMillis - prevPwrBootMillis) >= pwr_boot_interval) {
    // save the last time you blinked the LED
    prevPwrBootMillis = currentMillis;

    // START POWERCELL
    if (currentBootLevel != powerSeqTotal) {
      if (currentBootLevel == currentLightLevel) {
        if (currentLightLevel + 1 <= powerSeqTotal) {
          this->_lights->setPixelColor(currentLightLevel + 1, 0);
        }
        this->_lights->setPixelColor(currentBootLevel, this->_lights->Color(0, 0, 255));
        currentLightLevel = powerSeqTotal;
        currentBootLevel++;
      } else {
        if (currentLightLevel + 1 <= powerSeqTotal) {
          this->_lights->setPixelColor(currentLightLevel + 1, 0);
        }
        this->_lights->setPixelColor(currentLightLevel, this->_lights->Color(0, 0, 255));
        currentLightLevel--;
      }
      doUpdate = true;
    } else {
      powerBoot = true;
      currentBootLevel = powercellIndexOffset;
      currentLightLevel = powercellLedCount - powercellIndexOffset;
    }
    // END POWERCELL
  }

  if (doUpdate) {
    this->_lights->show();
  }
}

unsigned long prevPwrMillis = 0;  // last time we changed a powercell light in the idle sequence

void PowerCell::idle(unsigned long currentMillis, unsigned long anispeed) {
  bool doUpdate = false;

  // START POWERCELL
  if ((unsigned long)(currentMillis - prevPwrMillis) >= anispeed) {
    // save the last time you blinked the LED
    prevPwrMillis = currentMillis;

    for (int i = powercellIndexOffset; i <= powerSeqTotal; i++) {
      if (i <= powerSeqNum) {
        this->_lights->setPixelColor(i, this->_lights->Color(0, 0, 150));
      } else {
        this->_lights->setPixelColor(i, 0);
      }
    }

    if (powerSeqNum <= powerSeqTotal) {
      powerSeqNum++;
    } else {
      powerSeqNum = powercellIndexOffset;
    }

    doUpdate = true;
  }
  // END POWERCELL

  // if we changed anything update
  if (doUpdate == true) {
    this->_lights->show();
  }
}

bool animationComplete = false;

// FIXME: I think this is actually Overload
void PowerCell::off(bool start) {
  if(start) { 
    animationComplete = false;
    offTimer.begin(175);
  }

  if(animationComplete) return;

  if (offTimer.fire()) {
    for (int i = powerSeqTotal; i >= powercellIndexOffset; i--) {
      if (i <= powerShutdownSeqNum) {
        this->_lights->setPixelColor(i, this->_lights->Color(0, 0, 150));
      } else {
        this->_lights->setPixelColor(i, 0);
      }
    }

    this->_lights->show();

    if (powerShutdownSeqNum >= powercellIndexOffset) {
      powerShutdownSeqNum--;
    } else {
      powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
      animationComplete = true;
    }
  }
}


void PowerCell::clear() {
  powerSeqNum = powercellIndexOffset;
  powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
  currentLightLevel = powercellLedCount;
  currentBootLevel = powercellIndexOffset;
  powerBoot = false;
  animationComplete = false;

  this->_lights->clear();
  this->_lights->show();
}