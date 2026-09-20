# Agent guide

Two Arduino sketches, `MainPack/` and `NeutrinoWand/`, one per board. There is no test suite; the compiler is the only check, so every change is verified by compiling both sketches before and after and comparing the size lines.

## Compile check

Both boards are Arduino Unos. `Libraries/` holds the vendored HT16K33 driver; every other library comes from the standard Arduino libraries folder.

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries Libraries --warnings all MainPack
arduino-cli compile --fqbn arduino:avr:uno --libraries Libraries --warnings all NeutrinoWand
```

Each build ends with two lines, `Sketch uses N bytes` and `Global variables use N bytes`. Record both for each sketch on the base commit, then again after the change. Only warnings whose path is inside `MainPack/` or `NeutrinoWand/` matter; the AVR core emits its own.

What the numbers mean:

- A refactor that only removes dead code leaves RAM identical and flash equal or smaller.
- A refactor that moves code without changing it usually leaves both identical.
- A RAM change on a "no behavior change" diff is a signal to look again.

Quote the before and after numbers in the commit message or PR.

## Line endings

The `.ino`, `.cpp` and `.h` files use CRLF. Edit them with line endings preserved, or the diff shows every line changed and the review is worthless. After editing, `git diff --stat` should show a line count close to what you intended.
