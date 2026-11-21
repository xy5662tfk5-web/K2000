# K2000 asset intake on macOS

## Official downloads

The Kurzweil K2000 support page lists these files:

- [K2FACT.zip](https://kurzweil.com/wp-content/uploads/2019/10/K2FACT.zip) — original v1.3 K2000 factory programs and demos. Kurzweil places it under **Compatibility (Converter Files)**; do not assume it is a raw ROM image or a simple native preset bank.
- [K2KFARM.zip](https://kurzweil.com/wp-content/uploads/2019/10/K2KFARM.zip) — Base FARM, no special requirements. Kurzweil says FARM files are loaded into the instrument's RAM through Disk mode.
- [K2000 Series Musician's Guide ZIP](https://kurzweil.com/wp-content/uploads/2019/10/K2KMAN.zip) — owner manual.
- [K2000 support/download page](https://kurzweil.com/k2000/).

The same page says Orchestral FARM requires Orchestral ROM, Contemporary FARM requires Contemporary ROM, and K2VX FARM requires both option ROMs.

## Download and inventory

1. Download `K2FACT.zip` and `K2KFARM.zip` from the official links above into the Mac Downloads folder.
2. In Terminal, from the root of this repository, run:

   ```sh
   python3 tools/inspect_k2000_archives.py \
     ~/Downloads/K2FACT.zip \
     ~/Downloads/K2KFARM.zip \
     --output k2000-archive-report.json
   ```

3. The report records archive integrity, member paths, uncompressed/compressed sizes, CRC values, SHA-256 of each ZIP, and the first 32 bytes of each member. It does not alter the downloads or assume an undocumented format.
4. Share the generated `k2000-archive-report.json` or its text output. The report is metadata/signatures only; it does not contain the copyrighted sample data.

## Why inspect before implementing a parser?

The K2000 product page classifies K2FACT as a compatibility/converter download, while K2KFARM is an extra preset bank. The file extensions and internal object structure must be identified from the actual archive members before writing a parser. A program bank can refer to ROM keymaps and samples that are not included in the bank itself.

Kurzweil's FAQ also distinguishes battery-backed PRAM objects (programs, setups, QA banks, sequences) from volatile sample RAM. This confirms that preset/object data and waveform sample data are separate resources.

## Current limitation

The ZIP endpoints serve binary `application/zip` content. The available browsing/download route in this development session cannot retrieve those binary bodies, and the local shell has no external network access. The official links have been verified, but the ZIP contents have not yet been downloaded or parsed here.

## Next parser milestones

- Identify archive members and native file signatures from the inventory report.
- Read included loading/conversion notes.
- Build read-only format probes and test fixtures.
- Parse object headers and references before attempting program reconstruction.
- Implement waveform/sample loading and V.A.S.T. DSP as separate subsystems.
