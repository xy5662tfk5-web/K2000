# Kurzweil KRZ format research

Research date: 2026-11-21

## Status and confidence

A detailed public reverse-engineering reference is available at:
- https://hed0rah.github.io/audio_files_anatomy/krz-anatomy.html
- Reference implementation: https://github.com/hed0rah/acidcat/blob/main/src/acidcat/core/walk/krz.py
- Independent implementation cited by that reference: https://github.com/lentferj/mpc2emu (docs/KRZ_FORMAT.md)
- Hardware/SysEx object reference: https://github.com/psobot/k2000

The acidcat author states that its structural walker was verified against 242 real Sweetwater soundset banks. This is a strong external lead, but we have not yet run it against the user's K2KFARM files or independently verified the byte layout. Treat this document as a reverse-engineering map, not yet as our own validated parser specification.

## Container outline (PRAM banks)

The reference describes this layout:
1. 32-byte big-endian header beginning with ASCII PRAM.
2. A sequence of object blocks.
3. A big-endian signed 32-bit zero terminator.
4. A contiguous region of raw 16-bit big-endian PCM. Sample objects refer to this pool by word offset.

The header's signed big-endian value at offset 4 is described as the absolute byte offset of the PCM region (called osize in the reference). The software version is read at offset 16.

A separate SROM magic is used by some effects/sample-ROM-related files; it is not a PRAM object bank and must not be passed to the PRAM object walker.

## Object framing

Each PRAM object is described as:
- signed big-endian 32-bit negative block length;
- big-endian 16-bit packed type/id hash;
- big-endian 16-bit object size;
- big-endian 16-bit name offset;
- up to 16 ASCII bytes of object name plus alignment padding;
- type-specific body.

The proposed packed hash is (type << 10) | id, decoded as type = hash >> 10 and id = hash & 0x3ff.

Types called out as hardware-confirmed by the reference:
- 36: Program
- 37: Keymap
- 38: Sample

Other codes (Master, Studio/FX, Setup, etc.) are tentative and must remain opaque until corroborated.

The negative block length is used to advance to the next object. A robust reader must reject positive lengths, invalid ranges, integer overflow, out-of-file ranges, excessive object counts, and invalid PCM offsets.

## Object relationships

The documented object graph is Program -> Keymap -> Sample -> shared PCM region.
- A Program stores V.A.S.T. layer/segment data.
- A Keymap maps keyboard keys to Sample object IDs.
- A Sample describes playback metadata and points into the shared PCM data.

Keymap entry layout is not uniform in the cited material:
- method 0x03: 3-byte entry, sample ID (u16) + subsample (u8);
- method 0x13: 5-byte entry, tuning (i16) + sample ID (u16) + subsample (u8).

The entry stride must be read from the object; never hard-code one layout for all banks.

The Program body is described as tagged V.A.S.T. segments terminated by a 16-bit zero. Counting LYR tags can estimate layer count. However, the reference implementation explicitly says that the precise CAL-to-Keymap reference offset is not yet pinned down. We must not invent or decode that link until confirmed.

## Sample metadata and audio

The reference describes a 12-byte KSample header followed by one 32-byte Soundfilehead per channel. The sample period is a big-endian nanosecond period from which rate is derived:

rate = round(1,000,000,000 / samplePeriod)

The PCM pool is described as signed 16-bit big-endian audio. Sample start/loop/end positions are word offsets. The loop flag has an inverted bit convention in the reference. These details require fixture-level tests before being used in our engine.

## Engineering plan for this repository

1. Implement a defensive PRAM/SROM sniffer and header reader.
2. Implement a bounded object walker that reports object type, ID, name, offset and size without decoding unknown bodies.
3. Add Program, Keymap and Sample decoders incrementally, each with binary fixtures and bounds tests.
4. Add a graph report that resolves Program -> Keymap -> Sample references only after the Program CAL field is verified.
5. Add PCM extraction and byte-order tests independently of V.A.S.T. DSP.
6. Keep the file loader independent of the AU/Logic UI and independent of live MIDI SysEx transport.
7. Do not claim authentic synthesis from a successfully parsed preset bank: V.A.S.T. DSP, ROM waveform availability, envelopes, modulation and effects remain separate implementation work.

## Important limitation

The user has supplied archive member names and a lossy text rendering containing program-name strings, but has not yet supplied a complete .KRZ byte sequence to this repository's test environment. The next milestone is a parser implementation tested against a real lawful bank file, not merely a synthetic fixture or a strings scan.
