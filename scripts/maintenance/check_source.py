#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Reject staged/tracked files outside the reviewed source allowlist."""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[2]
allowed = set((root / 'packaging/source-files.txt').read_text().splitlines())
tracked = set(subprocess.check_output(['git', 'ls-files', '-z'], cwd=root).decode().split('\0')) - {''}
forbidden = sorted(tracked - allowed)
if forbidden:
    sys.exit('Restricted or unreviewed tracked files: ' + ', '.join(forbidden))
for name in allowed:
    path = root / name
    if path.is_symlink() or not path.is_file():
        sys.exit('Unsafe or missing approved source: ' + name)
print(f'Source gate passed: {len(tracked)} tracked; {len(allowed)} approved paths')
