#ifndef BarGraph_h
#define BarGraph_h
#include "Arduino.h"
#include <HT16K33.h>
#include <FireTimer.h>

class BarGraph {
public:
  BarGraph(uint8_t address = 0x70, uint8_t numberOfSegments = 28);

  void setup();
  void run();
  void reset();
  void clear(bool writeChanges = true);
  void volumeChanged(int volume);

  // Animations
  void boot(bool startAnimation = false);
  void cycle(bool startAnimation = false);
  void fire(bool startAnimation = false);
  void overload(bool startAnimation = false);
  void vent(bool startAnimation = false);
  void shutdown(bool startAnimation = false);

private:
  void write();
  void setSegment(uint8_t segmentNumber, uint8_t value);
  void _cycleBootStep(int boot);
  void _resetCycleAnimation();
  void _resetShutdownAnimation();
  void _resetFireAnimation();
  void _resetVentAnimation();
  uint8_t _address;
  uint8_t _numberOfSegments;
  HT16K33 _matrix;
  bool _isDisplayingVolume = false;
  FireTimer _volumeDisplayTimer;
  FireTimer _bootAnimationTimer;
  int _bootAnimationKeyframe = 0;
  FireTimer _cycleAnimationTimer;
  int _cycleAnimationKeyframe = 0;
  bool _cycleAnimationDirectionForward = true;
  FireTimer _shutdownAnimationTimer;
  int _shutdownAnimationKeyframe = 0;
  bool _shutdownAnimationDirectionForward = true;
  bool _shutdownAnimationComplete = false;
  FireTimer _fireAnimationTimer;
  int _fireAnimationKeyframe = 0;
  int _fireAnimationTimeout = 70;
  bool _fireAnimationDirectionForward = true;
  FireTimer _ventAnimationTimer;
  bool _ventAnimationAlternate = true;
};
#endif
