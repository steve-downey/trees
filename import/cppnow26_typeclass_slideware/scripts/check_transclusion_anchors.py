#!/usr/bin/env python3
"""Check that UUID-style transclusion anchors are unique and paired."""

from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
UUID_RE = re.compile(
    r"\s*//\s+([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})(\s+end)?\s*$"
)

starts: dict[str, pathlib.Path] = {}
ends: dict[str, pathlib.Path] = {}
errors: list[str] = []

for path in ROOT.rglob("*.cpp"):
    for line_no, line in enumerate(path.read_text().splitlines(), 1):
        match = UUID_RE.match(line)
        if not match:
            continue
        uuid = match.group(1)
        is_end = match.group(2) is not None
        table = ends if is_end else starts
        if uuid in table:
            errors.append(f"duplicate {'end' if is_end else 'start'} {uuid}: {path}:{line_no}")
        table[uuid] = path

for uuid, path in starts.items():
    if uuid not in ends:
        errors.append(f"missing end for {uuid}: {path}")

for uuid, path in ends.items():
    if uuid not in starts:
        errors.append(f"missing start for {uuid}: {path}")

if errors:
    for error in errors:
        print(error, file=sys.stderr)
    raise SystemExit(1)

print(f"checked {len(starts)} transclusion blocks")
