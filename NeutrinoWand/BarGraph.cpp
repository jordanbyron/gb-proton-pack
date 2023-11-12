#include "Arduino.h"
#include "BarGraph.h"
#include <HT16K33.h>
#include <FireTimer.h>

HT16K33 matrix = HT16K33();
bool isDisplayingVolume = false;
FireTimer volumeDisplayTimer;

BarGraph::BarGraph(uint8_t address = 0x70, uint8_t numberOfSegments = 28) {
  this->_address = address;
  this->_numberOfSegments = numberOfSegments;
}

void BarGraph::setup() {
  matrix.init(this->_address);
  delay(1000);
  matrix.setBrightness(10);
}

void BarGraph::run() {
  if (isDisplayingVolume && volumeDisplayTimer.fire(false)) {
    isDisplayingVolume = false;
    this->clear();
  }
}

void BarGraph::clear(bool writeChanges = true) {
  matrix.clear();
  if (writeChanges) matrix.write();
}

void BarGraph::volumeChanged(int volume) {
  isDisplayingVolume = true;
  volumeDisplayTimer.begin(2000);

  for (int i = 1; i <= this->_numberOfSegments; i++) {
    this->setSegment(i - 1, volume >= i ? 1 : 0);
  }
  this->write();
}

void BarGraph::write() {
  matrix.write();
}

void BarGraph::setSegment(uint8_t segmentNumber, uint8_t value) {
  uint8_t row, column;

  value &= 0x01;  //constrain val to be either 0 or 1

  // verify the position isn't greater than the bargraph size
  if (segmentNumber > (this->_numberOfSegments - 1)) segmentNumber = this->_numberOfSegments - 1;

  row = segmentNumber / 4;
  column = segmentNumber % 4;

  matrix.setPixel(row, column, value);
}

FireTimer bootAnimationTimer;
int bootAnimationKeyframe = 0;

void BarGraph::boot(bool startAnimation = false) {
  if (isDisplayingVolume) { return; }

  if (startAnimation) {
    bootAnimationTimer.begin(110);
    bootAnimationTimer.start();  // Force reset of timer
    bootAnimationKeyframe = 0;
  }

  if (startAnimation || bootAnimationTimer.fire()) {
    if (bootAnimationKeyframe < 28) {
      for (int i = 27; i >= 0; i--) {
        this->setSegment(i, (i >= (27 - bootAnimationKeyframe)));
      }
    } else {
      for (int i = 27; i >= 0; i--) {
        this->setSegment(i, (i < (27 - (bootAnimationKeyframe - 28))));
      }
    }
    this->write();
    if (bootAnimationKeyframe >= 28) {
      bootAnimationTimer.update(20);
    } else if (bootAnimationKeyframe == 56) {
      bootAnimationTimer.reset();
      bootAnimationKeyframe = 0;
    }
    bootAnimationKeyframe++;
  }
}

FireTimer cycleAnimationTimer;
int cycleAnimationKeyframe = 0;
bool cycleAnimationDirectionForward = true;

void resetCycleAnimation() {
  cycleAnimationTimer.begin(20);
  cycleAnimationKeyframe = 0;
  cycleAnimationDirectionForward = true;
}

void BarGraph::cycle(bool startAnimation = false) {
  if (isDisplayingVolume) { return; }

  if (startAnimation || cycleAnimationTimer.fire()) {
    if (cycleAnimationTimer.timeDiff >= 100) {
      resetCycleAnimation();
      return;
    }

    for (int i = 0; i < 28; i++) {
      this->setSegment(i, i <= cycleAnimationKeyframe);
    }

    this->write();
    if (cycleAnimationKeyframe == 27) {
      cycleAnimationDirectionForward = false;
    } else if (cycleAnimationKeyframe == 0) {
      cycleAnimationDirectionForward = true;
    }

    if (cycleAnimationDirectionForward) {
      cycleAnimationKeyframe++;
    } else {
      cycleAnimationKeyframe--;
    }
  }
}

FireTimer shutdownAnimationTimer;
int shutdownAnimationKeyframe = 0;
bool shutdownAnimationDirectionForward = true;
bool shutdownAnimationComplete = false;

void resetShutdownAnimation() {
  shutdownAnimationTimer.begin(10);
  shutdownAnimationKeyframe = 0;
  shutdownAnimationDirectionForward = true;
  shutdownAnimationComplete = false;
}

void BarGraph::shutdown(bool startAnimation = false) {
  if (isDisplayingVolume) { return; }

  if (startAnimation || (!shutdownAnimationComplete && shutdownAnimationTimer.fire())) {
    if (shutdownAnimationTimer.timeDiff >= 100) {
      resetShutdownAnimation();
      return;
    }

    for (int i = 0; i < 28; i++) {
      this->setSegment(i, i <= shutdownAnimationKeyframe);
    }

    this->write();
    if (shutdownAnimationKeyframe == 27) {
      shutdownAnimationDirectionForward = false;
      shutdownAnimationTimer.update(70);
    } else if (shutdownAnimationKeyframe == -1 && shutdownAnimationDirectionForward == false) {
      shutdownAnimationComplete = true;
      return;
    }

    if (shutdownAnimationDirectionForward) {
      shutdownAnimationKeyframe++;
    } else {
      shutdownAnimationKeyframe--;
    }
  }
}

FireTimer fireAnimationTimer;
int fireAnimationKeyframe = 0;
int fireAnimationTimeout = 70;
bool fireAnimationDirectionForward = true;
const int fireAnimationTop = 14;
const int fireAnimationBottom = 13;

void resetFireAnimation() {
  fireAnimationTimeout = 70;
  fireAnimationTimer.begin(fireAnimationTimeout);
  fireAnimationKeyframe = 0;
  fireAnimationDirectionForward = true;
}

void BarGraph::fire(bool startAnimation = false) {
  if (isDisplayingVolume) { return; }

  if (startAnimation || fireAnimationTimer.fire()) {
    if (fireAnimationTimer.timeDiff >= 100) {
      resetFireAnimation();
      return;
    }

    int topPixelOne = fireAnimationTop + fireAnimationKeyframe;
    int topPixelTwo = fireAnimationTop + 1 + fireAnimationKeyframe;

    int bottomPixelOne = fireAnimationBottom - fireAnimationKeyframe;
    int bottomPixelTwo = fireAnimationBottom - 1 - fireAnimationKeyframe;

    for (int i = 0; i < 27; i++) {
      this->setSegment(i, i == topPixelOne || i == topPixelTwo || i == bottomPixelOne || i == bottomPixelTwo);
    }

    this->write();
    if (fireAnimationKeyframe == 15) {
      fireAnimationKeyframe = 0;
      if (fireAnimationTimeout > 10) { fireAnimationTimeout -= 5; }
      fireAnimationTimer.update(fireAnimationTimeout);
    }

    if (fireAnimationDirectionForward) {
      fireAnimationKeyframe++;
    } else {
      fireAnimationKeyframe--;
    }
  }
}

FireTimer ventAnimationTimer;
const int ventAnimationTimeout = 500;
bool ventAnimationAlternate = true;

void resetVentAnimation() {
  ventAnimationTimer.begin(ventAnimationTimeout);
  ventAnimationAlternate = true;
}

void BarGraph::vent(bool startAnimation = false) {
  if (isDisplayingVolume) { return; }

  if (startAnimation || ventAnimationTimer.fire()) {
    if (startAnimation) { resetVentAnimation(); }

    this->clear(false);

    if (ventAnimationAlternate) {
      this->setSegment(9, 1);
      this->setSegment(10, 1);
      this->setSegment(11, 1);

      this->setSegment(17, 1);
      this->setSegment(18, 1);
      this->setSegment(19, 1);
    } else {
      this->setSegment(12, 1);
      this->setSegment(13, 1);

      this->setSegment(15, 1);
      this->setSegment(16, 1);
    }

    ventAnimationAlternate = !ventAnimationAlternate;

    this->write();
  }
}

void BarGraph::reset() {
  resetCycleAnimation();
  resetShutdownAnimation();
  resetFireAnimation();
  this->clear();
}