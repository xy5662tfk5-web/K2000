# Kurzweil K2000R AU

An independent macOS Audio Unit instrument project inspired by the Kurzweil K2000R.

## Project status

**Stage 0 — repository bootstrap.** This repository starts as an empty project. The first milestone is a real JUCE AU instrument shell with a K2000R-style front panel and functional navigation/state. The audio engine is a separate milestone; the interface must not imply that V.A.S.T. synthesis is already emulated.

## Planned milestones

1. **AU shell and front panel** — JUCE instrument target, LCD-style display, mode buttons, numeric keypad, data wheel, navigation and program selection.
2. **Program model and file I/O** — internal program representation, validated KRZ/K25 parsing research, bank browser and save/load.
3. **V.A.S.T. model** — layers, keymaps, DSP algorithm graph, envelopes, LFOs and modulation routing.
4. **DSP engine** — implement and validate processing blocks and signal flow.
5. **Effects and compatibility** — effects, MIDI behavior, presets, state recall and performance testing.

## Build prerequisites

- macOS
- CMake
- Xcode Command Line Tools
- JUCE 7 or 8 (provided through JUCE_DIR or CMake package discovery)

Example build commands:

    cmake -S . -B build -DJUCE_DIR=/path/to/JUCE
    cmake --build build --config Release

The first deliverable is an AU plugin shell. It is not yet a bit-accurate K2000 emulator and does not include Kurzweil ROM contents or proprietary firmware.
