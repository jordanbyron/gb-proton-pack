#include <StateMachine.h>
#include <Adafruit_NeoPixel.h>
#include <BfButton.h>
#include <FireTimer.h>
#include "VolumeControl.h"
#include "BarGraph.h"
#include "Lights.h"

StateMachine machine = StateMachine();

State* OFF = machine.addState(&off);
State* BOOTING = machine.addState(&booting);
State* LOCKED = machine.addState(&locked);
State* ACTIVATED = machine.addState(&activated);
State* FIRING = machine.addState(&firing);
State* OVERLOADING = machine.addState(&overloading);
State* VENTING = machine.addState(&venting);
State* POWERING_DOWN = machine.addState(&poweringDown);

bool isBooted = false;
bool isOverloading = false;
bool isPoweredDown = false;
bool isVented = false;

enum packMode {
  normal,
  volume,
  music
} packMode;

const int PIXEL_PIN = 6;
Lights lights(PIXEL_PIN);

const int NOSE_JEWEL_PIN = 7;
const int NOSE_JEWEL_COUNT = 7;
Adafruit_NeoPixel noseJewel(NOSE_JEWEL_COUNT, NOSE_JEWEL_PIN);

// Bargraph
const uint8_t BARGRAPH_SIZE = 28;

BarGraph barGraph(0x70, BARGRAPH_SIZE);

// **** Different Bargraph sequence modes **** //
enum barGraphSequences { BG_START,
                         BG_ACTIVE,
                         BG_FIRE1,
                         BG_FIRE2,
                         BG_VENT };
barGraphSequences BG_MODES;

// Switches & Buttons
const int STARTUP_SWITCH = 2;
const int SMOKE_ENABLED_SWITCH = 3;
const int SAFETY_SWITCH = 4;
const int FIRE_BUTTON = 5;

const int FRONT_KNOB_BTN = 8;
const int FRONT_KNOB_DT = 9;
const int FRONT_KNOB_CLK = 10;
BfButton frontKnobButton(BfButton::STANDALONE_DIGITAL, FRONT_KNOB_BTN);

const int INITIAL_VOLUME = 15;
VolumeControl volumeControl(FRONT_KNOB_DT, FRONT_KNOB_CLK, INITIAL_VOLUME, BARGRAPH_SIZE);

const int STATE_DELAY = 10;

// Serial Messages
const char MESSAGE_OFF = 'O';
const char MESSAGE_BOOT = 'B';
const char MESSAGE_LOCK_CYCLE = 'L';
const char MESSAGE_ACTIVATE_CYCLE = 'A';
const char MESSAGE_FIRE = 'f';
const char MESSAGE_OVERLOAD = 'F';
const char MESSAGE_VENT = 'V';
const char MESSAGE_POWER_DOWN = 'P';

const char MESSAGE_PLAY_PAUSE = 'M';
const char MESSAGE_PLAY_NEXT = 'm';

const char MESSAGE_PING = 'p';

FireTimer pingTimer;
unsigned long pingIntervalMillis = 500;

unsigned long currentMillis = 0;

void setup() {
  OFF->addTransition(&boot, BOOTING);

  BOOTING->addTransition(&cycleLocked, LOCKED);
  BOOTING->addTransition(&cycleActivated, ACTIVATED);

  LOCKED->addTransition(&cycleActivated, ACTIVATED);

  ACTIVATED->addTransition(&fire, FIRING);
  ACTIVATED->addTransition(&cycleLocked, LOCKED);

  FIRING->addTransition(&cycleActivated, ACTIVATED);
  FIRING->addTransition(&cycleLocked, LOCKED);
  FIRING->addTransition(&overloadWarning, OVERLOADING);

  OVERLOADING->addTransition(&vent, VENTING);

  VENTING->addTransition(&cycleAfterVent, ACTIVATED);

  POWERING_DOWN->addTransition(&offAfterPowerDown, OFF);

  // Powerdown transitions
  LOCKED->addTransition(&powerDown, POWERING_DOWN);
  ACTIVATED->addTransition(&powerDown, POWERING_DOWN);
  FIRING->addTransition(&powerDown, POWERING_DOWN);

  // Switches & Button Init
  pinMode(STARTUP_SWITCH, INPUT_PULLUP);
  pinMode(SMOKE_ENABLED_SWITCH, INPUT_PULLUP);
  pinMode(SAFETY_SWITCH, INPUT_PULLUP);
  pinMode(FIRE_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);

  lights.setup();

  noseJewel.begin();
  noseJewel.show();

  volumeControl.setup();
  volumeControl.onVolumeChange(volumeChanged);

  frontKnobButton.onPress(frontKnobPressed)
    .onDoublePress(frontKnobPressed);

  barGraph.setup();

  pingTimer.begin(pingIntervalMillis);
}

void loop() {
  currentMillis = millis();

  machine.run();
  volumeControl.run();
  barGraph.run();
  frontKnobButton.read();
  pingMainPack();

  delay(STATE_DELAY);
}

void pingMainPack() {
  if (pingTimer.fire()) {
    Serial.write(MESSAGE_PING);
  }
}

// ========= States ========

void off() {
  if (machine.executeOnce) {
    Serial.write(MESSAGE_OFF);
    isBooted = false;
    isPoweredDown = false;
    lights.clear();
    barGraph.reset();
  }
}

int bootDelay = 3700;
int bootStart;

FireTimer bootTimer;

void booting() {
  if (machine.executeOnce) {
    Serial.write(MESSAGE_BOOT);
    bootStart = millis();
    bootTimer.begin(bootDelay);
  }

  lights.boot(machine.executeOnce);
  barGraph.boot(machine.executeOnce);

  if (bootTimer.fire(false)) {
    isBooted = true;
  }
}

void locked() {
  if (machine.executeOnce) {
    Serial.write(MESSAGE_LOCK_CYCLE);
  }

  lights.locked(machine.executeOnce);
  barGraph.cycle(machine.executeOnce);
}

void activated() {
  // Cycle Stuff + Wand lights
  if (machine.executeOnce) {
    Serial.write(MESSAGE_ACTIVATE_CYCLE);
    clearFireStrobe();
  }

  lights.activated(machine.executeOnce);
  barGraph.cycle(machine.executeOnce);
}

int overloadDelay = 10000;
FireTimer overloadTimer;

void firing() {
  if (machine.executeOnce) {
    overloadDelay = isFastOverloadSwitchOn() ? 5000 : 10000;
    Serial.write(MESSAGE_FIRE);
    isOverloading = false;
    overloadTimer.begin(overloadDelay);
    barGraph.reset();
  };

  lights.activated(machine.executeOnce);
  barGraph.fire(machine.executeOnce);
  fireStrobe(currentMillis);

  if (overloadTimer.fire()) isOverloading = true;
}

void overloading() {
  if (machine.executeOnce) {
    Serial.write(MESSAGE_OVERLOAD);
  };

  lights.overload(machine.executeOnce);
  barGraph.fire(machine.executeOnce);
  fireStrobe(currentMillis);
}

const int ventDuration = 3500;
FireTimer ventTimer;

void venting() {
  if (machine.executeOnce) {
    Serial.write(MESSAGE_VENT);
    ventTimer.begin(ventDuration);
    clearFireStrobe();
  };

  lights.boot(machine.executeOnce);
  barGraph.vent(machine.executeOnce);

  // SMOKE ... maybe
  if (ventTimer.fire()) {
    isVented = true;
  }
}

int powerDownDelay = 3000;
FireTimer powerDownTimer;

void poweringDown() {
  // Fun animations and sounds
  if (machine.executeOnce) {
    Serial.write(MESSAGE_POWER_DOWN);
    powerDownTimer.begin(powerDownDelay);
  }

  lights.locked(machine.executeOnce);
  barGraph.shutdown(machine.executeOnce);

  if (powerDownTimer.fire(false)) {
    isPoweredDown = true;
  }
}

// ======== Transitions =======

bool boot() {
  return isStartupSwitchOn() && isBooted == false;
}

bool cycleLocked() {
  return isBooted && !isSafetySwitchOn();
}

bool cycleActivated() {
  return isBooted && isSafetySwitchOn() && !isFireButtonOn();
}

bool fire() {
  return isFireButtonOn();
}

bool overloadWarning() {
  return isOverloading && isFireButtonOn();
}

bool vent() {
  return !isFireButtonOn();
}

bool cycleAfterVent() {
  if (isVented) {
    isVented = false;
    return true;
  } else {
    return false;
  }
}

bool powerDown() {
  return !isStartupSwitchOn();
}

bool offAfterPowerDown() {
  return isPoweredDown;
}

// ============ Switch Helpers ============
// Switches are set to pull up, so they are HIGH when off, and LOW when on
// Connect one pole of the switch to the input and the other to GND

bool isStartupSwitchOn() {
  return digitalRead(STARTUP_SWITCH) == LOW;
}

bool isSafetySwitchOn() {
  return digitalRead(SAFETY_SWITCH) == LOW;
}

bool isFireButtonOn() {
  return digitalRead(FIRE_BUTTON) == LOW;
}

bool isFastOverloadSwitchOn() {
  return digitalRead(SMOKE_ENABLED_SWITCH) == LOW;
}

// ========= Front Knob ==================

void frontKnobPressed(BfButton* btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.write(MESSAGE_PLAY_PAUSE);
      break;

    case BfButton::DOUBLE_PRESS:
      Serial.write(MESSAGE_PLAY_NEXT);
      break;
  }
}

void volumeChanged(int volume) {
  Serial.write(volume);
  barGraph.volumeChanged(volume);
}

/*************** Nose Jewel Firing Animations *********************/
unsigned long prevFireMillis = 0;
const unsigned long fire_interval = 50;  // interval at which to cycle lights (milliseconds).
int fireSeqNum = 0;
int fireSeqTotal = 5;

void clearFireStrobe() {
  for (int i = 0; i < 7; i++) {
    noseJewel.setPixelColor(i, 0);
  }
  noseJewel.show();
  fireSeqNum = 0;
}

void fireStrobe(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevFireMillis) >= fire_interval) {
    prevFireMillis = currentMillis;

    switch (fireSeqNum) {
      case 0:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(2, 0);
        noseJewel.setPixelColor(3, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(4, 0);
        noseJewel.setPixelColor(5, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(6, 0);
        break;
      case 1:
        noseJewel.setPixelColor(0, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 2:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(1, 0);
        noseJewel.setPixelColor(2, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(3, 0);
        noseJewel.setPixelColor(4, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(5, 0);
        noseJewel.setPixelColor(6, noseJewel.Color(255, 0, 0));
        break;
      case 3:
        noseJewel.setPixelColor(0, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 4:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(1, 0);
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, 0);
        noseJewel.setPixelColor(4, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(5, 0);
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 5:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(0, 255, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(3, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 0, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(6, noseJewel.Color(0, 0, 255));
        break;
    }

    noseJewel.show();

    fireSeqNum++;
    if (fireSeqNum > fireSeqTotal) {
      fireSeqNum = 0;
    }
  }
}

// Copyright (c) 2023 Jordan Byron

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.