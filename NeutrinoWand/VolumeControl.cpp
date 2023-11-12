#include "Arduino.h"
#include "VolumeControl.h"

VolumeControl::VolumeControl(uint16_t dt, uint16_t clock, int currentVolume = 20, int maxVolume = 28, int minVolume = 0) {
  this->_dtPin = dt;
  this->_clockPin = clock;
  this->_maxVolume = maxVolume;
  this->_minVolume = minVolume;
  this->_volume = currentVolume;
}

void VolumeControl::setup() {
  pinMode(this->_clockPin, INPUT_PULLUP);
  pinMode(this->_dtPin, INPUT_PULLUP);

  this->_clockLastState = digitalRead(this->_clockPin);
}

void VolumeControl::run() {
  int clockState = digitalRead(this->_clockPin);
  int currentVolume = this->_volume;

  if (clockState == HIGH && this->_clockLastState == LOW) {
    if (digitalRead(this->_dtPin) == HIGH) {
      currentVolume--;
    } else {
      currentVolume++;
    }
    if (currentVolume >= this->_maxVolume) {
      currentVolume = this->_maxVolume;
    }
    if (currentVolume <= this->_minVolume) {
      currentVolume = this->_minVolume;
    }

    if (currentVolume != this->_volume) {
      this->_volume = currentVolume;
      if (this->_onVolumeChangeCallback) this->_onVolumeChangeCallback(this->_volume);
    }
  }

  this->_clockLastState = clockState;
}

void VolumeControl::onVolumeChange(callback_t_volume_change callback) {
  this->_onVolumeChangeCallback = callback;
}