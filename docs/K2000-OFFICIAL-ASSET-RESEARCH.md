# K2000 official asset research

Research date: 2026-11-21

Primary source: https://kurzweil.com/k2000/

## Verified official downloads

| File | Official description | Notes for this project |
|---|---|---|
| K2FACT.zip | Original v1.3 K2000 Factory Programs and Demos (316 KB) | Native factory object/program data candidate; not a raw sample-ROM dump. |
| K2KFARM.zip | Base FARM, no special requirements (151 KB) | Extra preset/object bank. Kurzweil says FARM files are loaded into RAM through Disk mode. |
| OPRGSDEM.zip | Orchestral FARM (170 KB) | Requires Orchestral ROM. |
| CPRGSDEM.zip | Contemporary FARM (182 KB) | Requires Contemporary ROM. |
| K2VXFARM.zip | K2VX FARM (218 KB) | Requires both Orchestral and Contemporary ROMs. |
| K2000 Series Musician's Guide | Owner's manual (listed as 2 MB) | Needed to verify object types, load/save procedures, UI pages and parameter meanings. |

Direct official URLs:
- https://kurzweil.com/wp-content/uploads/2019/10/K2FACT.zip
- https://kurzweil.com/wp-content/uploads/2019/10/K2KFARM.zip
- https://kurzweil.com/wp-content/uploads/2019/10/K2KMAN.zip

## Important distinction

These downloads are program/object banks and documentation. They are not evidence of the factory Sample ROM binary contents. The K2000 product page separately identifies Base ROM, Contemporary ROM Option, K2VX and Orchestral ROM Option in its audio section, but the download list above does not provide raw ROM images.

Kurzweil's page explicitly says FARM files must be loaded into RAM via Disk mode. It also identifies the Orchestral and Contemporary dependencies for the corresponding FARM banks.

## Current acquisition status

The official page and download metadata have been inspected. The ZIP endpoints return binary ZIP content that this research environment cannot retrieve for extraction, so no archive contents have yet been parsed or validated. Do not claim a parser is complete or that any preset data has been imported until the actual ZIP bytes are available.

## Next engineering steps

1. Obtain the official K2FACT and K2KFARM ZIP bytes in a form the build environment can read.
2. Inspect archive members and identify the native Kurzweil object-file format and any included readme/conversion instructions.
3. Create a small parser/test fixture from lawful example files; record object IDs, names, dependencies and references.
4. Determine which object types contain program definitions versus keymaps, samples and other dependencies.
5. Implement the loader only after the file layout and checksums/lengths are understood.
6. Keep sample playback and V.A.S.T. DSP separate from preset parsing. A preset bank alone does not provide the factory waveform ROM.

## Primary source references

- K2000 official product/download page: https://kurzweil.com/k2000/
- K2FACT: https://kurzweil.com/wp-content/uploads/2019/10/K2FACT.zip
- K2KFARM: https://kurzweil.com/wp-content/uploads/2019/10/K2KFARM.zip
- Musician's Guide: https://kurzweil.com/wp-content/uploads/2019/10/K2KMAN.zip


## K2KFARM archive member names supplied by the user

The following list was transcribed from the user's inspection of the extracted K2KFARM archive. It is useful as an intake manifest, but the individual files have not yet been independently opened or parsed by the project tooling.

| Archive member | User-supplied description |
|---|---|
| `ANACOMPS.KRZ` | Analog accompaniments and polyphonic synthesizers |
| `ANALEADS.KRZ` | Analog lead sounds |
| `ANAPADS.KRZ` | Analog pads |
| `BASS.KRZ` | Bass sounds |
| `BELLS.KRZ` | Bells and metallic/percussive sounds |
| `BRASS.KRZ` | Brass instruments |
| `DIGITAL.KRZ` | Digital synthesizer sounds |
| `ENSEMBLE.KRZ` | Ensemble sounds |
| `ETHEREAL.KRZ` | Atmospheric and texture sounds |
| `GUITARS.KRZ` | Guitar sounds |
| `HYBRIDS.KRZ` | Hybrid/combined sounds |
| `HYBPERC.KRZ` | Hybrid percussion |
| `K2KFACTO.KRZ` | Factory programs |
| `ORGANS.KRZ` | Organ sounds |
| `PNOEPNO.KRZ` | Acoustic and electric pianos |
| `STRINGS.KRZ` | String instruments |

### Parser priorities from this inventory

1. Start with `K2KFACTO.KRZ` to establish a baseline factory-program import test.
2. Use `ANALEADS.KRZ`, `ANACOMPS.KRZ`, `DIGITAL.KRZ` and `HYBRIDS.KRZ` as representative synth-program test cases.
3. Include `GUITARS.KRZ` and `PNOEPNO.KRZ` to test multisample/keymap references and velocity/key-range structures.
4. Do not assume that the .KRZ objects contain the underlying factory sample audio. Resolve program-to-keymap/sample references separately against the K2000 ROM/sample resources.

## External engineering lead: psobot/k2000

Research date: 2026-11-21

A public MIT-licensed Python project, [psobot/k2000](https://github.com/psobot/k2000), provides a substantial reference implementation for communicating with K2000/K2500/K2600 instruments over MIDI SysEx.

Confirmed capabilities from its README and source:
- reads and writes Kurzweil object data through SysEx;
- exposes object proxies for Programs, Keymaps, Effects, Setups and other object types;
- implements SysEx messages for object directory/info, dump, load, read, write and bank-read operations;
- can retrieve a program name and raw program data from a connected instrument.

Important limitation: this is an instrument communication library, **not a demonstrated offline KRZ file parser**. It cannot be dropped into the AU plugin as a file reader without further work. Its value is as a documented reference for Kurzweil object types, binary object payloads, and SysEx transport. Its README lists Python 3.8–3.10 and dependencies including NumPy, Pillow, attrs and MIDI support, so it is not automatically a lightweight dependency for the plugin.

A separate utility, K2KList, is described by Sweetwater as listing the objects in a K2000 disk file with object size and ID. This is a promising historical reference for offline file inventory, but its binary and source availability still need to be checked before relying on it.

### Consequence for our implementation

Do not copy the SysEx transport layer into the AU plugin. Instead:
1. Treat psobot/k2000 as a reference for object type IDs, metadata and object payload handling.
2. Continue investigating the offline KRZ container independently.
3. Reuse only format details that can be confirmed against the official manual or actual bank bytes.
4. Keep the future plugin's file parser and its MIDI/SysEx integration as separate modules.

Sources:
- https://github.com/psobot/k2000
- https://github.com/psobot/k2000/blob/main/k2000/messages.py
- https://github.com/psobot/k2000/blob/main/k2000/client.py
- https://www.sweetwater.com/sweetcare/kurzweil-k2000-sounds-and-software/

## Historical offline inventory utility: K2KList

A Sweetwater archive page documents two old K2KList utilities:
- PC archive: `k2klist.zip` (8 KB), described as listing all objects in a K2000 disk file with object size and ID.
- Mac utility: `k2klist.mac` (51 KB), also described as listing all objects stored in a K2000 disk file.

The page also links to a short PC text file and a Mac "about" file. The direct Mac binary endpoint was identified, but its contents were not successfully retrieved or inspected during this research pass. Compatibility with current macOS is unknown. Do not assume this 1990s utility runs on a modern Mac.

This is a stronger lead for the offline disk-file/object inventory problem than a generic printable-string scan. Next research task is to obtain and inspect the small K2KList documentation/binary (or a preserved source/disassembly) and compare its behavior with the modern psobot/k2000 SysEx object model.

Source page:
- https://www.sweetwater.com/sweetcare/kurzweil-k2000-sounds-and-software/
- PC utility: https://www.sweetwater.com/k2000/software/pc/k2klist.zip
- PC description: https://www.sweetwater.com/k2000/software/pc/k2klist.txt
- Mac utility: https://www.sweetwater.com/k2000/software/mac/k2klist.mac
- Mac description: https://www.sweetwater.com/k2000/software/mac/k2klist.about
