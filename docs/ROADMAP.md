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
- [ ] Obtain at least one actual .KRZ file byte-for-byte for project test environment
- [x] Identify psobot/k2000 as a SysEx/object-payload reference (not an offline KRZ parser)
- [x] Identify historical K2KList PC/Mac offline object-inventory utilities
- [x] Add a standard-library printable-string scanner with byte offsets (reconnaissance only)
- [x] Find a detailed external PRAM/SROM and object-layout reference plus a readable KRZ walker implementation
- [x] Document tentative object framing, Program/Keymap/Sample relationships, and parsing caveats
- [ ] Independently validate the external layout against a real user-supplied K2KFARM .KRZ file
- [ ] Implement bounded PRAM/SROM detection and object inventory in this project
- [ ] Add Program, Keymap and Sample decoders with binary fixtures and bounds tests
- [ ] Verify Program CAL-to-Keymap references before resolving the full object graph
- [ ] Model programs, layers, keymaps, algorithms and modulation routes

## Stage 3 — synthesis
- [ ] Implement V.A.S.T. DSP building blocks
- [ ] Validate envelopes, key tracking, velocity response and modulation
- [ ] Add sample playback and sample-rate conversion
- [ ] Add effects and MIDI controller support

## Accuracy policy
The current oscillator is only a build/audition placeholder. It is not Kurzweil synthesis. UI labels and parameter ranges remain provisional until verified against primary documentation and test files. External format references are research leads until independently validated against our own real bank fixtures.
