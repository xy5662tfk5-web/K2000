# KRZ printable-string reconnaissance

The repository includes `tools/scan_krz_strings.py`, a small Python 3 standard-library utility for scanning an extracted Kurzweil object/bank file.

It reports:
- file size and SHA-256 (to identify the exact input examined);
- printable ASCII text runs and their byte offsets (decimal and hexadecimal);
- the raw bytes for each text run;
- optional JSON output for later comparison and automated analysis.

It does **not** decode KRZ objects, identify record boundaries, or prove that a string is a program name. A string offset is a clue for subsequent binary inspection, not an object address.

## Run on macOS

Download only the script (no repository clone required):

```sh
curl -L https://raw.githubusercontent.com/xy5662tfk5-web/K2000/main/tools/scan_krz_strings.py -o ~/Downloads/scan_krz_strings.py
```

Then scan one extracted file:

```sh
python3 ~/Downloads/scan_krz_strings.py ~/Downloads/K2KFACTO.KRZ
```

For a machine-readable report:

```sh
python3 ~/Downloads/scan_krz_strings.py ~/Downloads/K2KFACTO.KRZ --json > ~/Downloads/K2KFACTO-strings.json
```

If the bank contains legacy high-bit text, try the optional Latin-1 pass:

```sh
python3 ~/Downloads/scan_krz_strings.py ~/Downloads/K2KFACTO.KRZ --latin1
```

Keep the original file unchanged. When sharing results, the JSON/text report plus the exact filename and SHA-256 are enough for the first analysis pass; the full bank does not need to be uploaded for this initial step.
