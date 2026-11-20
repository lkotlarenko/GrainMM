#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Fetch and extract the audited SDK locally; no system installation or overwrites."""
import hashlib
from pathlib import Path
import subprocess
import tarfile

root = Path(__file__).resolve().parents[2]
destination = root / '.local/sdk'
archive = destination / 'miyoomini-toolchain.tar.xz'
expected = 'e158b6ebde0c933ecd57b36760fb8602fad4176a09fb61559dc7bcffe6624f2e'
destination.mkdir(parents=True, exist_ok=True)
if not archive.exists():
    subprocess.run(['gh', 'release', 'download', 'v0.0.3', '-R',
                    'shauninman/miyoomini-toolchain-buildroot', '-p', archive.name,
                    '-D', str(destination)], check=True)
with archive.open('rb') as stream:
    digest = hashlib.file_digest(stream, 'sha256').hexdigest()
if digest != expected:
    raise SystemExit('SDK checksum mismatch; refusing extraction')
target = destination / 'miyoomini-toolchain'
if target.exists():
    print('Verified cached archive; existing SDK left untouched:', target)
else:
    with tarfile.open(archive) as source:
        source.extractall(destination, filter='data')
    print('Verified SDK extracted:', target)
