#include "Arduino.h"
#include "VolumeControl.h"

// Both encoder pins must be on port B (D8 - D13), which shares PCINT0_vect
static VolumeControl* encoderInstance;

ISR(PCINT0_vect) {
  if (encoderInstance) encoderInstance->handlePinChangeInterrupt();
}

static const int8_t QUADRATURE_STEP_BY_TRANSITION[16] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };

const uint8_t KY040_DETENT_STATE = 0b11;

// A detent is four transitions; accept two so one lost while NeoPixel show() blocks interrupts doesn't drop the click
const int8_t MIN_TRANSITIONS_PER_DETENT = 2;

VolumeControl::VolumeControl(uint16_t dt, uint16_t clock, int currentVolume, int maxVolume, int minVolume) {
  this->_dtPin = dt;
  this->_clockPin = clock;
  this->_maxVolume = maxVolume;
  this->_minVolume = minVolume;
  this->_volume = currentVolume;
}

void VolumeControl::setup() {
  pinMode(this->_clockPin, INPUT_PULLUP);
  pinMode(this->_dtPin, INPUT_PULLUP);

  this->_encoderState = this->_readEncoder();
  encoderInstance = this;

  *digitalPinToPCMSK(this->_clockPin) |= bit(digitalPinToPCMSKbit(this->_clockPin));
  *digitalPinToPCMSK(this->_dtPin) |= bit(digitalPinToPCMSKbit(this->_dtPin));
  PCICR |= bit(digitalPinToPCICRbit(this->_clockPin));
}

uint8_t VolumeControl::_readEncoder() {
  return (digitalRead(this->_clockPin) << 1) | digitalRead(this->_dtPin);
}

void VolumeControl::handlePinChangeInterrupt() {
  uint8_t newState = this->_readEncoder();

  this->_transitions += QUADRATURE_STEP_BY_TRANSITION[(this->_encoderState << 2) | newState];
  this->_encoderState = newState;

  if (newState == KY040_DETENT_STATE) {
    if (this->_transitions >= MIN_TRANSITIONS_PER_DETENT) this->_steps++;
    if (this->_transitions <= -MIN_TRANSITIONS_PER_DETENT) this->_steps--;
    this->_transitions = 0;
  }
}

void VolumeControl::run() {
  noInterrupts();
  int8_t steps = this->_steps;
  this->_steps = 0;
  interrupts();

  if (steps == 0) return;

  int currentVolume = constrain(this->_volume + steps, this->_minVolume, this->_maxVolume);

  if (currentVolume != this->_volume) {
    this->_volume = currentVolume;
    if (this->_onVolumeChangeCallback) this->_onVolumeChangeCallback(this->_volume);
  }
}

void VolumeControl::onVolumeChange(callback_t_volume_change callback) {
  this->_onVolumeChangeCallback = callback;
}