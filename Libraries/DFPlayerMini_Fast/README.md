# DFPlayerMini_Fast (vendored)

Copy of the DFPlayerMini_Fast 1.2.4 release from the Arduino Library Manager
(https://github.com/PowerBroker2/DFPlayerMini_Fast), with one change: the
debug `Serial.print` string literals are wrapped in `F()` so they live in
flash instead of RAM.

On the Uno those strings cost about 280 bytes of RAM once `currentVolume()`
links in the response parser, which is most of the headroom the pack has.
Upstream master made the same `F()` change after the 1.2.4 release but also
changed the parameter byte sent by `playNext`, `pause`, `resume` and others,
so the release code is kept here rather than tracking master.

Only `src/` and `library.properties` are vendored. The `--libraries Libraries`
flag in the compile command makes this copy take priority over any copy in
the Arduino libraries folder.
