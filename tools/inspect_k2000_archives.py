#!/usr/bin/env python3
"""Inventory official Kurzweil K2000 ZIP downloads without modifying them.

Usage:
  python3 tools/inspect_k2000_archives.py ~/Downloads/K2FACT.zip ~/Downloads/K2KFARM.zip

The report records archive integrity, member names, sizes, CRC status, SHA-256,
and a short hexadecimal signature. It deliberately does not guess proprietary
object layouts or extract copyrighted sample content.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import sys
import zipfile


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def inspect(path: pathlib.Path) -> dict:
    result: dict = {
        "archive": str(path),
        "exists": path.is_file(),
    }
    if not path.is_file():
        result["error"] = "File not found"
        return result

    result["size_bytes"] = path.stat().st_size
    result["sha256"] = sha256_file(path)
    with path.open("rb") as stream:
        result["header_hex"] = stream.read(32).hex(" ")

    if not zipfile.is_zipfile(path):
        result["is_zip"] = False
        result["note"] = "Not recognized as a ZIP archive; inspect format manually."
        return result

    result["is_zip"] = True
    members = []
    with zipfile.ZipFile(path) as archive:
        bad_member = archive.testzip()
        result["zip_integrity"] = "ok" if bad_member is None else "failed"
        if bad_member is not None:
            result["first_corrupt_member"] = bad_member

        for info in archive.infolist():
            entry = {
                "name": info.filename,
                "size_bytes": info.file_size,
                "compressed_bytes": info.compress_size,
                "crc32": f"{info.CRC:08x}",
                "is_directory": info.is_dir(),
            }
            if not info.is_dir():
                with archive.open(info) as member:
                    entry["first_32_bytes_hex"] = member.read(32).hex(" ")
            members.append(entry)
    result["members"] = members
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archives", nargs="+", type=pathlib.Path)
    parser.add_argument("--output", type=pathlib.Path, help="Write JSON report to this path")
    args = parser.parse_args()

    report = {
        "tool": "inspect_k2000_archives.py",
        "purpose": "Archive inventory only; no proprietary format assumptions",
        "results": [inspect(path) for path in args.archives],
    }
    rendered = json.dumps(report, indent=2, ensure_ascii=False)
    if args.output:
        args.output.write_text(rendered + "\n", encoding="utf-8")
    print(rendered)
    return 0 if all(item.get("zip_integrity") == "ok" for item in report["results"] if item.get("is_zip")) else 1


if __name__ == "__main__":
    sys.exit(main())
