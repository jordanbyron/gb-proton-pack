#include <StateMachine.h>
#include <Adafruit_NeoPixel.h>
#include <FireTimer.h>
#include <BfButton.h>
#include "PowerCell.h"
#include "Cyclotron.h"

// for the sound board
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>

const int NEOPIXEL_POWER_CELL_PIN = 7;
const int NEOPIXEL_POWER_CELL_COUNT = 15;
PowerCell powerCell = PowerCell::PowerCell(NEOPIXEL_POWER_CELL_COUNT, NEOPIXEL_POWER_CELL_PIN);

const int NEOPIXEL_CYCLOTRON_PIN = 6;
Cyclotron cyclotronAndVent = Cyclotron::Cyclotron(NEOPIXEL_CYCLOTRON_PIN, 0, 1, 4, 8);

// Smoke pins
const int SMOKE = 4;
const int FAN = 5;
const int smokeDelay = 5000;  // Half of *overloadDelay* in Wand config
FireTimer smokeFireTimer;

// Soundboard pins and setup
const int SFX_RX = 8;
const int SFX_TX = 9;
const int ACT = 10;  // this allows us to know if the audio is playing
const int INITIAL_VOLUME = 15;

SoftwareSerial sfxSerial = SoftwareSerial(SFX_RX, SFX_TX);
DFPlayerMini_Fast sfx;

// ======== Debug / Standalone Mode =========

const int DEBUG_BTN = 2;
int debugIndex = 0;
BfButton debugButton(BfButton::STANDALONE_DIGITAL, DEBUG_BTN);

// =========== State Machine (Coppied from Wand sketch) ===============

StateMachine machine = StateMachine();

State* OFF = machine.addState(&off);
State* BOOTING = machine.addState(&booting);
State* LOCKED = machine.addState(&locked);
State* ACTIVATED = machine.addState(&activated);
State* FIRING = machine.addState(&firing);
State* OVERLOADING = machine.addState(&overloading);
State* VENTING = machine.addState(&venting);
State* POWERING_DOWN = machine.addState(&poweringDown);

// ============= Audio State Machine ================

StateMachine audioMachine = StateMachine();

State* SFX = audioMachine.addState(&sfxMode);
State* MUSIC = audioMachine.addState(&musicMode);

const int STATE_DELAY = 10;

// ================ Serial Messages (Coppied from Wand sketch) =============

const char MESSAGE_OFF = 'O';
const char MESSAGE_BOOT = 'B';
const char MESSAGE_LOCK_CYCLE = 'L';
const char MESSAGE_ACTIVATE_CYCLE = 'A';
const char MESSAGE_FIRE = 'f';
const char MESSAGE_OVERLOAD = 'F';
const char MESSAGE_VENT = 'V';
const char MESSAGE_POWER_DOWN = 'P';

const int STATE_MESSAGES[] = { MESSAGE_BOOT, MESSAGE_LOCK_CYCLE, MESSAGE_ACTIVATE_CYCLE, MESSAGE_FIRE, MESSAGE_OVERLOAD, MESSAGE_VENT, MESSAGE_ACTIVATE_CYCLE, MESSAGE_POWER_DOWN, MESSAGE_OFF };

const char MESSAGE_PLAY_PAUSE = 'M';
const char MESSAGE_PLAY_NEXT = 'm';

const char MESSAGE_PING = 'p';

// ======= Wand Connectivity =======

FireTimer wandConnectedTimer;
unsigned long wandCheckIntervalMillis = 1000;
bool wandConnected = false;

unsigned long playIdleTrackAtMillis = 0;
bool musicPlaying = false;
char lastMessage;
unsigned long currentMillis = 0;

void setup() {
  OFF->addTransition(&boot, BOOTING);

  BOOTING->addTransition(&cycleLocked, LOCKED);
  BOOTING->addTransition(&cycleActivated, ACTIVATED);

  LOCKED->addTransition(&cycleActivatedFromLock, ACTIVATED);

  ACTIVATED->addTransition(&fire, FIRING);
  ACTIVATED->addTransition(&cycleLockedFromActivated, LOCKED);

  FIRING->addTransition(&cycleActivatedFromFire, ACTIVATED);
  FIRING->addTransition(&cycleLocked, LOCKED);
  FIRING->addTransition(&overloadWarning, OVERLOADING);

  OVERLOADING->addTransition(&vent, VENTING);

  VENTING->addTransition(&cycleAfterVent, ACTIVATED);

  POWERING_DOWN->addTransition(&offAfterPowerDown, OFF);

  // Powerdown transitions
  LOCKED->addTransition(&powerDown, POWERING_DOWN);
  ACTIVATED->addTransition(&powerDown, POWERING_DOWN);
  FIRING->addTransition(&powerDown, POWERING_DOWN);

  // =========== Audio Transitions ==========

  SFX->addTransition(&playMusic, MUSIC);
  MUSIC->addTransition(&pauseMusic, SFX);
  MUSIC->addTransition(&nextSong, MUSIC);

  // Software serial for the audio board
  sfxSerial.begin(9600);
  sfxSerial.listen();
  // Hardward serial for the Neutrino Wand
  Serial.begin(115200);

  // set act modes for the fx board
  pinMode(ACT, INPUT);

  // set smoke pins
  pinMode(SMOKE, OUTPUT);
  pinMode(FAN, OUTPUT);

  digitalWrite(SMOKE, LOW);
  digitalWrite(FAN, LOW);

  powerCell.setup();
  cyclotronAndVent.setup();

  if (!sfx.begin(sfxSerial, true)) {
    Serial.println("Not found");
    while (1)
      ;
  }

  delay(1000);

  Serial.println("SFX board found");
  sfx.wakeUp();
  sfx.startDAC();

  delay(500);

  volumeChanged(INITIAL_VOLUME);

  debugButton.onPress(debugButtonPressed).onDoublePress(debugButtonPressed).onPressFor(debugButtonPressed, 2000);

  wandConnectedTimer.begin(wandCheckIntervalMillis);
}

void loop() {
  currentMillis = millis();

  checkWandConnectivity();
  fetchMessageFromWand();

  debugButton.read();

  audioMachine.run();
  machine.run();

  //lastMessage = ""; // Clear last message

  delay(STATE_DELAY);
}

void fetchMessageFromWand() {
  if (Serial.available() > 0) {
    if (Serial.peek() == MESSAGE_PING) {
      Serial.read();
      return;
    }

    Serial.print("Message from wand ");

    if (isAlpha(Serial.peek())) {
      lastMessage = Serial.read();
      Serial.println(lastMessage);
    } else {
      int volume = Serial.read();
      Serial.print("- Volume ");
      Serial.println(volume);
      volumeChanged(volume);
    }
  }
}

void checkWandConnectivity() {
  bool previousState = wandConnected;

  if (Serial.available() > 0) {
    if (debugIndex > 0) exitDebugMode();
    debugIndex = 0;
    wandConnected = true;
    wandConnectedTimer.start();
  } else if (wandConnectedTimer.fire()) {
    wandConnected = false;
  }

  if (wandConnected != previousState) {
    Serial.print("Wand ");
    Serial.println(wandConnected ? "Connected" : "Disconnected");
  }
}

void debugButtonPressed(BfButton* btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      lastMessage = STATE_MESSAGES[debugIndex];
      Serial.println(debugIndex);
      debugIndex++;
      if (debugIndex > 8) {
        debugIndex = 0;
      }
      break;

    case BfButton::DOUBLE_PRESS:
      lastMessage = MESSAGE_PLAY_PAUSE;
      break;
    case BfButton::LONG_PRESS:
      exitDebugMode();
      break;
  }
}

void exitDebugMode() {
  lastMessage = "";
  machine.transitionTo(OFF);
  debugIndex = 0;
}

// ========= States ========

void off() {
  if (machine.executeOnce) {
    powerCell.clear();
    cyclotronAndVent.clear();
    stopSfx();
    Serial.println("Off");
    setSmoke(false);
    setFan(false);
  }
}

void booting() {
  if (machine.executeOnce) {
    playSfx(1);
    Serial.println("Booting");
  }

  powerCell.boot(currentMillis);
  cyclotronAndVent.boot(currentMillis);
}

void locked() {
  powerCell.idle(currentMillis, 60);
  cyclotronAndVent.idle(currentMillis, 1000);

  if (machine.executeOnce) {
    Serial.println("Cycling (locked)");
  }

  //if(!audioPlaying()) playSfx(2); // Idle
  playIdleTrack();
}

void activated() {
  powerCell.idle(currentMillis, 60);
  cyclotronAndVent.idle(currentMillis, 1000);

  if (machine.executeOnce) {
    Serial.println("Cycling (activated)");
    setSmoke(false);
    setFan(false);
  }

  playIdleTrack();
  //if(!audioPlaying()) playSfx(2); // Idle
}

void firing() {
  if (machine.executeOnce) {
    playSfx(11);  // Fire
    smokeFireTimer.begin(smokeDelay);
  }

  if (!audioPlaying()) playSfx(11);  // Fire

  powerCell.idle(currentMillis, 60);
  cyclotronAndVent.idle(currentMillis, 1000);

  if (smokeFireTimer.fire(false)) {
    setSmoke(true);
  };
}

void overloading() {
  if (machine.executeOnce) {
    playSfx(16);  // Warning
    setSmoke(true);
  }

  if (!audioPlaying()) playSfx(16);  // Warning

  powerCell.idle(currentMillis, 20);
  cyclotronAndVent.idle(currentMillis, 200);
}

void venting() {
  if (machine.executeOnce) {
    playSfx(9);  // Vent
    playIdleTrack(3390);
    cyclotronAndVent.vent(currentMillis);
    powerCell.clear();
    setSmoke(false);
    setFan(true);
  }

  powerCell.boot(currentMillis);
  cyclotronAndVent.boot(currentMillis);
}

void poweringDown() {
  if (machine.executeOnce) {
    Serial.println("Powering Down");
    playSfx(10);
    setSmoke(false);
    setFan(false);
  }

  powerCell.off(machine.executeOnce);
  cyclotronAndVent.off(currentMillis);
}

// ============ Transition Conditions =================

bool boot() {
  return lastMessage == MESSAGE_BOOT;
}

bool cycleLocked() {
  return lastMessage == MESSAGE_LOCK_CYCLE;
}

bool cycleLockedFromActivated() {
  bool shouldTransition = lastMessage == MESSAGE_LOCK_CYCLE;

  if (shouldTransition) {
    playSfx(8);  // Click
    playIdleTrack(150);
  }

  return shouldTransition;
}

bool cycleActivatedFromLock() {
  bool shouldTransition = lastMessage == MESSAGE_ACTIVATE_CYCLE;

  if (shouldTransition) {
    playSfx(7);  // Charge
    playIdleTrack(1500);
  }

  return shouldTransition;
}

bool cycleActivatedFromFire() {
  bool shouldTransition = lastMessage == MESSAGE_ACTIVATE_CYCLE;

  if (shouldTransition) {
    playSfx(4);  // Fire tail
    playIdleTrack(1720);
  }

  return shouldTransition;
}

bool cycleActivated() {
  return lastMessage == MESSAGE_ACTIVATE_CYCLE;
}

bool fire() {
  return lastMessage == MESSAGE_FIRE;
}

bool overloadWarning() {
  return lastMessage == MESSAGE_OVERLOAD;
}

bool vent() {
  return lastMessage == MESSAGE_VENT;
}

bool cycleAfterVent() {
  bool shouldTransition = lastMessage == MESSAGE_ACTIVATE_CYCLE || lastMessage == MESSAGE_LOCK_CYCLE;

  if (shouldTransition) cyclotronAndVent.clear();

  return shouldTransition;
}

bool powerDown() {
  return lastMessage == MESSAGE_POWER_DOWN;
}

bool offAfterPowerDown() {
  return lastMessage == MESSAGE_OFF;
}

// ========= Audio States =========

void sfxMode() {
  if (audioMachine.executeOnce) {
    lastMessage = "";
    musicPlaying = false;
    if (audioPlaying()) sfx.stop();
  }
}

void musicMode() {
  if (audioMachine.executeOnce) {
    lastMessage = "";
    musicPlaying = true;
    sfx.repeatFolder(1);
  }
}

// ======== Audio Transitions =========

bool playMusic() {
  return lastMessage == MESSAGE_PLAY_PAUSE;
}

bool pauseMusic() {
  return lastMessage == MESSAGE_PLAY_PAUSE;
}

bool nextSong() {
  bool transition = musicPlaying && lastMessage == MESSAGE_PLAY_NEXT;

  if (transition) {
    lastMessage = "";
    sfx.playNext();
  }
  return transition;
}

// ========= SFX Utils ==========

unsigned long previousPlayMillis;
const int IS_PLAYING_LAG_MILLIS = 500;

void stopSfx() {
  if (musicPlaying) return;

  playIdleTrackAtMillis = 0;

  sfx.stop();
}

void playSfx(int trackNumber) {
  if (musicPlaying) return;

  sfx.play(trackNumber);
  previousPlayMillis = currentMillis;
}

void loopSfx(int trackNumber) {
  if (musicPlaying) return;

  sfx.loop(trackNumber);
}

void playIdleTrack() {
  if (currentMillis > playIdleTrackAtMillis) {
    playSfx(2);
    playIdleTrackAtMillis = currentMillis + 33190;
  }
}

void playIdleTrack(int delayMillis) {
  playIdleTrackAtMillis = delayMillis + currentMillis;

  playIdleTrack();
}

bool audioPlaying() {
  if (digitalRead(ACT) == LOW) return true;
  if (currentMillis < (previousPlayMillis + IS_PLAYING_LAG_MILLIS)) return true;

  return false;
}

void volumeChanged(int volume) {
  //sfxSerial.listen(); Unnecessary Listen?
  sfx.volume(volume);
}

void setSmoke(bool smokeOn) {
  digitalWrite(SMOKE, smokeOn ? HIGH : LOW);
}

void setFan(bool fanOn) {
  digitalWrite(FAN, fanOn ? HIGH : LOW);
}
