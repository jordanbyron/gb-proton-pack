# GhostBusters Arduino Proton Pack

Yet another Arduino based Proton Pack project. This variant uses state machines to simplify run loops and make it much
easier to understand state transition and functionality. Heavily inspired by and using code snippets from both [CountDeMonet](https://github.com/CountDeMonet/ArduinoProtonPack) and [arpehem](https://github.com/arpehem/Arpehem-s-proton-pack-code)'s projects.

## Wiring

This setup requires two Arduinos: One inside the main pack and the other inside the neutrona wand. They'll communicate
via a serial connection. Wiring for each Arduino is below:

### Pack GPIO

| Pin #  | Device                               |
| ------ | ------------------------------------ |
| 0 (RX) | Wand 1 (TX)                          |
| 1 (TX) | Wand 0 (RX)                          |
| 2      | Test Button (WIP)                    |
| 3      |                                      |
| 4      | Smoke                                |
| 5      | Fan                                  |
| 6      | Cyclotron / Vent Neopixels           |
| 7      | Powercell Neopixels                  |
| 8      | Audio RX (To Audio TX)               |
| 9      | Audio TX (To Audio RX - w/ Resistor) |
| 10     | Audio Busy                           |
| 11     |                                      |
| 12     |                                      |
| 13     |                                      |

### Wand GPIO

| Pin # | Device |
| --- | --- |
| 0 (RX) | Pack 1 (TX) |
| 1 (TX) | Pack 0 (RX) |
| 2 | Switch (Boot - Bottom Main Wand Body) |
| 3 | Switch (Top Main Body) |
| 4 | Switch (Activate / Safety) |
| 5 | Button (Firing)  |
| 6 | Lights Neopixels |
| 7 | Nose Jewel Neopixel |
| 8 | Button (Front knob) blue  |
| 9 | DT (Front knob) yellow  |
| 10 | CLK (Front knob) green |
| 11 |  |
| 12 |  |
| 13 |  |
| A4 | SDA (Bargraph) |
| A5 | SCL (Bargraph) |

### 27 Segment Bargraph

| Bargraph | HT16k33 Breakout Board |
| -------- | ---------------------- |
| 5V | VDD |
| GND | GND |
| Pin 21 | C0 |
| Pin 15 | C1 |
| Pin 13 | C2 |
| Pin 16 | C3 |
| Pin 22 | A0 |
| Pin 1 | A1 |
| Pin 19 | A2 |
| Pin 18 | A3 |
| Pin 7 | A4 |
| Pin 10 | A5 |
| Pin 11 | A6 |
