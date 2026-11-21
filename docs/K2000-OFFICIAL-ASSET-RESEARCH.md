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
