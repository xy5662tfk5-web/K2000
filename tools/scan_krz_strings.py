#!/usr/bin/env python3
"""Extract printable text runs from a Kurzweil K2000 object/bank file.

This is a reconnaissance tool, not a KRZ decoder. It reports byte offsets for
human-readable strings and makes no assumptions about object records.

Usage:
  python3 scan_krz_strings.py ~/Downloads/K2KFARM/K2KFACTO.KRZ
  python3 scan_krz_strings.py bank.KRZ --min-length 5 --json
  python3 scan_krz_strings.py bank.KRZ --latin1

Only Python's standard library is required. The input file is never modified.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import string
import sys


ASCII_PRINTABLE = set(bytes(string.printable, "ascii")) - {0x0b, 0x0c, 0x0d, 0x0a, 0x09}


def is_printable(value: int, latin1: bool) -> bool:
    return value in ASCII_PRINTABLE or (latin1 and 0xA0 <= value <= 0xFF)


def scan(data: bytes, minimum: int, latin1: bool) -> list[dict]:
    found: list[dict] = []
    start: int | None = None

    def finish(end: int) -> None:
        nonlocal start
        if start is None:
            return
        raw = data[start:end]
        if len(raw) >= minimum:
            encoding = "latin-1" if latin1 and any(byte >= 0xA0 for byte in raw) else "ascii"
            found.append({
                "offset": start,
                "offset_hex": f"0x{start:08X}",
                "length": len(raw),
                "text": raw.decode(encoding, errors="replace"),
                "bytes_hex": raw.hex(" "),
            })
        start = None

    for index, value in enumerate(data):
        if is_printable(value, latin1):
            if start is None:
                start = index
        else:
            finish(index)
    finish(len(data))
    return found


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", type=pathlib.Path, help="Extracted .KRZ or other binary file")
    parser.add_argument("--min-length", type=int, default=4, help="Minimum run length (default: 4)")
    parser.add_argument("--latin1", action="store_true", help="Also recognize bytes 0xA0-0xFF")
    parser.add_argument("--json", action="store_true", help="Print machine-readable JSON")
    parser.add_argument("--no-hex", action="store_true", help="Omit raw bytes column in text output")
    args = parser.parse_args()

    if args.min_length < 1:
        parser.error("--min-length must be at least 1")
    if not args.file.is_file():
        print(f"error: file not found: {args.file}", file=sys.stderr)
        return 2

    data = args.file.read_bytes()
    strings = scan(data, args.min_length, args.latin1)
    report = {
        "tool": "scan_krz_strings.py",
        "input": str(args.file),
        "size_bytes": len(data),
        "sha256": __import__("hashlib").sha256(data).hexdigest(),
        "minimum_run_length": args.min_length,
        "latin1_enabled": args.latin1,
        "strings_found": len(strings),
        "strings": strings,
        "warning": "Printable strings only; offsets do not identify object boundaries or prove record structure.",
    }

    if args.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print(f"File: {args.file}")
        print(f"Size: {len(data)} bytes | SHA-256: {report['sha256']}")
        print(f"Printable runs: {len(strings)} (minimum length {args.min_length})")
        print("-" * 78)
        for item in strings:
            suffix = "" if args.no_hex else f"  [{item['bytes_hex']}]"
            print(f"{item['offset_hex']}  {item['text']}{suffix}")
        print("-" * 78)
        print("Note: strings are clues only; this scan does not decode KRZ objects.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
