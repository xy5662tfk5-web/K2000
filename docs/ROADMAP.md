# Development roadmap

## Stage 0 — bootstrap
- [x] Create standalone repository and project documentation
- [ ] Compile JUCE AU shell on Intel macOS
- [ ] Validate AU discovery in Logic Pro

## Stage 1 — hardware-inspired front panel
- [x] K2000R-style panel layout
- [x] LCD-like program display
- [x] Mode buttons, soft keys, numeric keypad
- [x] Program stepping, layer selection, data wheel
- [ ] Replace provisional labels with verified K2000 page names and workflows
- [ ] Add file chooser and bank/program browser

## Stage 2 — formats and model
- [x] Record official K2KFARM archive member inventory supplied by the user
- [x] Document distinction between preset/object banks and factory sample ROM
- [ ] Obtain at least one actual `.KRZ` file byte-for-byte for format inspection
- [ ] Identify container/header/object records from real file bytes; do not infer layout from extension or filename
- [ ] Build parser tests from user-supplied lawful example files
- [ ] Model programs, layers, keymaps, algorithms and modulation routes

## Stage 3 — synthesis
- [ ] Implement V.A.S.T. DSP building blocks
- [ ] Validate envelopes, key tracking, velocity response and modulation
- [ ] Add sample playback and sample-rate conversion
- [ ] Add effects and MIDI controller support

## Accuracy policy
The current oscillator is only a build/audition placeholder. It is not Kurzweil synthesis. UI labels and parameter ranges remain provisional until verified against primary documentation and test files.
