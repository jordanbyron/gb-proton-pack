#include "Arduino.h"
#include "BarGraph.h"
#include <HT16K33.h>
#include <FireTimer.h>

BarGraph::BarGraph(uint8_t address, uint8_t numberOfSegments) {
  this->_address = address;
  this->_numberOfSegments = numberOfSegments;
}

void BarGraph::setup() {
  this->_matrix.init(this->_address);
  delay(1000);
  this->_matrix.setBrightness(10);
}

void BarGraph::run() {
  if (this->_isDisplayingVolume && this->_volumeDisplayTimer.fire(false)) {
    this->_isDisplayingVolume = false;
    this->clear();
  }
}

void BarGraph::clear(bool writeChanges) {
  this->_matrix.clear();
  if (writeChanges) this->_matrix.write();
}

void BarGraph::volumeChanged(int volume) {
  this->_isDisplayingVolume = true;
  this->_volumeDisplayTimer.begin(2000);

  for (int i = 1; i <= this->_numberOfSegments; i++) {
    this->setSegment(i - 1, volume >= i ? 1 : 0);
  }
  this->write();
}

void BarGraph::write() {
  this->_matrix.write();
}

void BarGraph::setSegment(uint8_t segmentNumber, uint8_t value) {
  uint8_t row, column;

  value &= 0x01;  //constrain val to be either 0 or 1

  // verify the position isn't greater than the bargraph size
  if (segmentNumber > (this->_numberOfSegments - 1)) segmentNumber = this->_numberOfSegments - 1;

  row = segmentNumber / 4;
  column = segmentNumber % 4;

  this->_matrix.setPixel(row, column, value);
}

void BarGraph::boot(bool startAnimation) {
  if (this->_isDisplayingVolume) { return; }

  if (startAnimation) {
    this->_bootAnimationTimer.begin(110);
    this->_bootAnimationTimer.start();  // Force reset of timer
    this->_bootAnimationKeyframe = 0;
  }

  if (startAnimation || this->_bootAnimationTimer.fire()) {
    if (this->_bootAnimationKeyframe < 28) {
      for (int i = 27; i >= 0; i--) {
        this->setSegment(i, (i >= (27 - this->_bootAnimationKeyframe)));
      }
    } else {
      for (int i = 27; i >= 0; i--) {
        this->setSegment(i, (i < (27 - (this->_bootAnimationKeyframe - 28))));
      }
    }
    this->write();
    if (this->_bootAnimationKeyframe >= 28) {
      this->_bootAnimationTimer.update(20);
    } else if (this->_bootAnimationKeyframe == 56) {
      this->_bootAnimationTimer.reset();
      this->_bootAnimationKeyframe = 0;
    }
    this->_bootAnimationKeyframe++;
  }
}

void BarGraph::_resetCycleAnimation() {
  this->_cycleAnimationTimer.begin(20);
  this->_cycleAnimationKeyframe = 0;
  this->_cycleAnimationDirectionForward = true;
}

void BarGraph::cycle(bool startAnimation) {
  if (this->_isDisplayingVolume) { return; }

  if (startAnimation || this->_cycleAnimationTimer.fire()) {
    if (this->_cycleAnimationTimer.timeDiff >= 100) {
      this->_resetCycleAnimation();
      return;
    }

    for (int i = 0; i < 28; i++) {
      this->setSegment(i, i <= this->_cycleAnimationKeyframe);
    }

    this->write();
    if (this->_cycleAnimationKeyframe == 27) {
      this->_cycleAnimationDirectionForward = false;
    } else if (this->_cycleAnimationKeyframe == 0) {
      this->_cycleAnimationDirectionForward = true;
    }

    if (this->_cycleAnimationDirectionForward) {
      this->_cycleAnimationKeyframe++;
    } else {
      this->_cycleAnimationKeyframe--;
    }
  }
}

void BarGraph::_resetShutdownAnimation() {
  this->_shutdownAnimationTimer.begin(10);
  this->_shutdownAnimationKeyframe = 0;
  this->_shutdownAnimationDirectionForward = true;
  this->_shutdownAnimationComplete = false;
}

void BarGraph::shutdown(bool startAnimation) {
  if (this->_isDisplayingVolume) { return; }

  if (startAnimation || (!this->_shutdownAnimationComplete && this->_shutdownAnimationTimer.fire())) {
    if (this->_shutdownAnimationTimer.timeDiff >= 100) {
      this->_resetShutdownAnimation();
      return;
    }

    for (int i = 0; i < 28; i++) {
      this->setSegment(i, i <= this->_shutdownAnimationKeyframe);
    }

    this->write();
    if (this->_shutdownAnimationKeyframe == 27) {
      this->_shutdownAnimationDirectionForward = false;
      this->_shutdownAnimationTimer.update(70);
    } else if (this->_shutdownAnimationKeyframe == -1 && this->_shutdownAnimationDirectionForward == false) {
      this->_shutdownAnimationComplete = true;
      return;
    }

    if (this->_shutdownAnimationDirectionForward) {
      this->_shutdownAnimationKeyframe++;
    } else {
      this->_shutdownAnimationKeyframe--;
    }
  }
}

const int fireAnimationTop = 14;
const int fireAnimationBottom = 13;

void BarGraph::_resetFireAnimation() {
  this->_fireAnimationTimeout = 70;
  this->_fireAnimationTimer.begin(this->_fireAnimationTimeout);
  this->_fireAnimationKeyframe = 0;
  this->_fireAnimationDirectionForward = true;
}

void BarGraph::fire(bool startAnimation) {
  if (this->_isDisplayingVolume) { return; }

  if (startAnimation || this->_fireAnimationTimer.fire()) {
    if (this->_fireAnimationTimer.timeDiff >= 100) {
      this->_resetFireAnimation();
      return;
    }

    int topPixelOne = fireAnimationTop + this->_fireAnimationKeyframe;
    int topPixelTwo = fireAnimationTop + 1 + this->_fireAnimationKeyframe;

    int bottomPixelOne = fireAnimationBottom - this->_fireAnimationKeyframe;
    int bottomPixelTwo = fireAnimationBottom - 1 - this->_fireAnimationKeyframe;

    for (int i = 0; i < 27; i++) {
      this->setSegment(i, i == topPixelOne || i == topPixelTwo || i == bottomPixelOne || i == bottomPixelTwo);
    }

    this->write();
    if (this->_fireAnimationKeyframe == 15) {
      this->_fireAnimationKeyframe = 0;
      if (this->_fireAnimationTimeout > 10) { this->_fireAnimationTimeout -= 5; }
      this->_fireAnimationTimer.update(this->_fireAnimationTimeout);
    }

    if (this->_fireAnimationDirectionForward) {
      this->_fireAnimationKeyframe++;
    } else {
      this->_fireAnimationKeyframe--;
    }
  }
}

const int ventAnimationTimeout = 500;

void BarGraph::_resetVentAnimation() {
  this->_ventAnimationTimer.begin(ventAnimationTimeout);
  this->_ventAnimationAlternate = true;
}

void BarGraph::vent(bool startAnimation) {
  if (this->_isDisplayingVolume) { return; }

  if (startAnimation || this->_ventAnimationTimer.fire()) {
    if (startAnimation) { this->_resetVentAnimation(); }

    this->clear(false);

    if (this->_ventAnimationAlternate) {
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

    this->_ventAnimationAlternate = !this->_ventAnimationAlternate;

    this->write();
  }
}

void BarGraph::reset() {
  this->_resetCycleAnimation();
  this->_resetShutdownAnimation();
  this->_resetFireAnimation();
  this->clear();
}