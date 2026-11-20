#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Verify private baseline inputs remain intact; run only in the original workspace."""
import hashlib
import json
from pathlib import Path
import sys
import tarfile

root = Path(__file__).resolve().parents[2]
base = root / '.local/baseline'
manifest = json.loads((base / 'sha256.json').read_text())
relocation_file = base / 'relocations.json'
relocations = json.loads(relocation_file.read_text()) if relocation_file.exists() else {}
with tarfile.open(base / 'reference-and-history.tar.gz') as backup:
    original_readme = backup.extractfile('README.md').read()
failures = []
for name, digest in manifest.items():
    path = root / relocations.get(name, name)
    if not path.is_file():
        failures.append(name)
    elif name == 'README.md' and name not in relocations:
        if not path.read_bytes().endswith(original_readme):
            failures.append(name)
    elif hashlib.sha256(path.read_bytes()).hexdigest() != digest:
        failures.append(name)
if failures:
    sys.exit('Changed or missing reference inputs: ' + ', '.join(failures))
print(f'Preserved {len(manifest)} baseline input files; original README preserved (reference copy or legacy suffix)')
