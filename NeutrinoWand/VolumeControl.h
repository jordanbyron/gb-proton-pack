#ifndef VolumeControl_h
#define VolumeControl_h
#include "Arduino.h"

class VolumeControl {
public:
  VolumeControl(uint16_t dt, uint16_t clock, int currentVolume = 20, int maxVolume = 28, int minVolume = 0);

  typedef void (*callback_t_volume_change)(int volume);

  void setup(void);
  void run(void);
  void onVolumeChange(callback_t_volume_change);

  void handlePinChangeInterrupt(void);

private:
  uint16_t _dtPin;
  uint16_t _clockPin;
  int _maxVolume;
  int _minVolume;
  int _volume;
  uint8_t _encoderState;
  int8_t _transitions = 0;
  volatile int8_t _steps = 0;
  uint8_t _readEncoder();
  void _updateVolume();
  callback_t_volume_change _onVolumeChangeCallback;
};
#endif