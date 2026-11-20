#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Create matching source from the reviewed allowlist, never Git history or private inputs."""
import argparse
import gzip
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import tarfile

ROOT = Path(__file__).resolve().parents[2]


def source_payload(root=ROOT):
    names = (root / 'packaging/source-files.txt').read_text().splitlines()
    if len(names) != len(set(names)):
        raise ValueError('duplicate source allowlist entries')
    payload = {}
    prohibited = {'.local', '.git', '_vendor', 'GrainMM_Codex_Master_Prompt.md',
                  'GrainMM_Feasibility_Research.md'}
    for name in sorted(names):
        parts = PurePosixPath(name)
        if not name or str(parts) != name or '\\' in name or ':' in name or parts.is_absolute() or '..' in parts.parts or prohibited.intersection(parts.parts):
            raise ValueError('private or unsafe source path')
        path = root / name
        if any((root / Path(*parts.parts[:i])).is_symlink() for i in range(1, len(parts.parts) + 1)):
            raise ValueError('symlink source path')
        if not path.is_file() or path.suffix in {'.wav', '.so', '.zip', '.log', '.bin'}:
            raise ValueError('missing or forbidden source input')
        payload[name] = path.read_bytes()
    if 'LICENSES/GPL-3.0-only.txt' not in payload or 'LICENSE_SCOPE.md' not in payload:
        raise ValueError('GPL/source scope notice missing')
    return payload


def build_source(output, version, root=ROOT):
    metadata = json.loads((root / 'packaging/release.json').read_text())
    if version != metadata['version']:
        raise ValueError('source version must match release metadata')
    payload = source_payload(root)
    manifest = {'version': version, 'license': 'GPL-3.0-only',
                'sha256': {name: hashlib.sha256(data).hexdigest() for name, data in payload.items()}}
    payload['SOURCE_MANIFEST.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    prefix = f'GrainMM-{version}-source'
    buffer = io.BytesIO()
    with gzip.GzipFile(fileobj=buffer, filename='', mode='wb', mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w') as archive:
            for name, data in sorted(payload.items()):
                member = tarfile.TarInfo(prefix + '/' + name)
                member.size = len(data)
                member.mode = 0o755 if name.endswith('.sh') else 0o644
                member.mtime = 0
                archive.addfile(member, io.BytesIO(data))
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open('xb') as stream:
        stream.write(buffer.getvalue())
    with output.with_suffix(output.suffix + '.sha256').open('x') as stream:
        stream.write(hashlib.sha256(buffer.getvalue()).hexdigest() + '  ' + output.name + '\n')
    print(f'Validated matching source: {len(payload) - 1} reviewed files, {len(buffer.getvalue())} bytes')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', required=True)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    # Reuse the release version policy without making network calls.
    from package import release_version
    version = release_version(args.version)
    build_source(args.output or ROOT / f'dist/GrainMM-{version}-source.tar.gz', version)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError) as error:
        raise SystemExit(f'Source package rejected: {error}') from error
