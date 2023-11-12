#ifndef BarGraph_h
#define BarGraph_h
#include "Arduino.h"

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
  uint8_t _address;
  uint8_t _numberOfSegments;
};
#endif
