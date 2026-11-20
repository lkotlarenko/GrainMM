#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Verify versioned installation and exact matching source against the release checkout."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import sys
import tarfile
import zipfile
from package import ROOT, validate_archive, validate_version
from source_archive import source_payload


def verify(directory, version, root=ROOT):
    validate_version(version)
    metadata = json.loads((root / 'packaging/release.json').read_text())
    if metadata['version'] != version:
        raise ValueError('release version does not match checkout')
    install = directory / f'GrainMM-{version}-miyoo-mini-plus.zip'
    source = directory / f'GrainMM-{version}-source.tar.gz'
    for artifact in (install, source):
        if artifact.is_symlink() or not artifact.is_file():
            raise ValueError('release asset must be a regular file')
        checksum = artifact.with_suffix(artifact.suffix + '.sha256')
        if checksum.is_symlink() or not checksum.is_file():
            raise ValueError('missing safe checksum file')
        expected = hashlib.sha256(artifact.read_bytes()).hexdigest() + '  ' + artifact.name
        if checksum.read_text().strip() != expected:
            raise ValueError('release asset checksum mismatch')
    validate_archive(install)
    with zipfile.ZipFile(install) as archive:
        info = json.loads(archive.read('App/GrainMM/MANIFEST.json'))
        if info.get('version') != version or info.get('status') != 'RELEASE CANDIDATE':
            raise ValueError('installation version/status mismatch')
    payload = source_payload(root)
    expected_hashes = {name: hashlib.sha256(data).hexdigest() for name, data in payload.items()}
    prefix = f'GrainMM-{version}-source/'
    with tarfile.open(fileobj=io.BytesIO(source.read_bytes()), mode='r:gz') as archive:
        members = archive.getmembers()
        names = [m.name for m in members]
        expected_names = {prefix + name for name in payload} | {prefix + 'SOURCE_MANIFEST.json'}
        if len(names) != len(set(names)) or set(names) != expected_names:
            raise ValueError('source asset paths do not match checkout allowlist')
        if any(not m.isfile() or m.size > 4 * 1024 * 1024 for m in members):
            raise ValueError('unsafe source asset member')
        manifest = json.load(archive.extractfile(prefix + 'SOURCE_MANIFEST.json'))
        if (manifest.get('version') != version or manifest.get('license') != 'GPL-3.0-only' or
                manifest.get('sha256') != expected_hashes):
            raise ValueError('source manifest does not match release checkout')
        for name, digest in expected_hashes.items():
            if hashlib.sha256(archive.extractfile(prefix + name).read()).hexdigest() != digest:
                raise ValueError('source asset content mismatch')
    print(f'Verified {version}: installation/checksums and {len(payload)} matching source files')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', required=True)
    parser.add_argument('--directory', type=Path, default=ROOT / 'dist')
    args = parser.parse_args()
    verify(args.directory, args.version)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError, tarfile.TarError, zipfile.BadZipFile) as error:
        sys.exit(f'Release verification rejected: {error}')
