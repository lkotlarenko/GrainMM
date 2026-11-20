#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build a deterministic, allowlisted Onion installation ZIP; never bundle the reference tree."""
import argparse
import hashlib
import io
import json
import re
from pathlib import Path
import struct
import sys
import zipfile
import zlib

sys.path.insert(0, str(Path(__file__).resolve().parent))
from importlib.util import module_from_spec, spec_from_file_location

ROOT = Path(__file__).resolve().parents[2]
FILES = {
    'App/GrainMM/grainmm': 'build/miyoo-armv7-release/grainmm',
    'App/GrainMM/launch.sh': 'packaging/onion/launch.sh',
    'App/GrainMM/config.json': 'packaging/onion/config.json',
    'App/GrainMM/diagnose-miyoo.sh': 'scripts/miyoo/diagnose.sh',
    'App/GrainMM/EXPERIMENTAL.txt': 'packaging/onion/EXPERIMENTAL.txt',
    'App/GrainMM/CREDITS.txt': 'packaging/onion/CREDITS.txt',
    'App/GrainMM/LICENSE_SCOPE.md': 'LICENSE_SCOPE.md',
    'App/GrainMM/GPL-3.0-only.txt': 'LICENSES/GPL-3.0-only.txt',
    'App/GrainMM/SOURCE_NOTICE.txt': 'packaging/onion/SOURCE_NOTICE.txt',
    'App/GrainMM/GrainMM-MIT.txt': 'LICENSES/GrainMM-MIT.txt',
    'App/GrainMM/INSTALLATION.md': 'docs/INSTALLATION.md',
    'App/GrainMM/USER_GUIDE.md': 'docs/PARAMETERS.md',
    'App/GrainMM/TROUBLESHOOTING.md': 'docs/TROUBLESHOOTING.md',
}


def icon():
    # Original studio mark: charcoal panel, burgundy grains and a mint signal dot.
    # Four samples per axis keep the small launcher image legible without assets.
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))

    def rounded(x, y, left, top, right, bottom, radius):
        if not left <= x <= right or not top <= y <= bottom:
            return False
        dx = max(left + radius - x, 0, x - (right - radius))
        dy = max(top + radius - y, 0, y - (bottom - radius))
        return dx * dx + dy * dy <= radius * radius

    bars = [(12, 28, 16, 37), (21, 22, 25, 43), (30, 17, 34, 48),
            (39, 24, 43, 41), (48, 29, 52, 36)]

    def pixel(x, y):
        color = (0, 0, 0, 0)
        if rounded(x, y, 2, 2, 62, 62, 11):
            color = (64, 70, 80, 255)
        if rounded(x, y, 3, 3, 61, 61, 10):
            color = (18, 21, 28, 255)
        if 12 <= x <= 39 and 10 <= y <= 12:
            color = (231, 229, 215, 255)
        if 12 <= x <= 52 and 53 <= y <= 54:
            color = (64, 70, 80, 255)
        if any(rounded(x, y, *bar, 2) for bar in bars):
            color = (218, 84, 118, 255)
        if (x - 51) ** 2 + (y - 11) ** 2 <= 2.2 ** 2:
            color = (106, 218, 206, 255)
        return color

    rows = bytearray()
    offsets = (.125, .375, .625, .875)
    for y in range(64):
        rows.append(0)
        for x in range(64):
            samples = [pixel(x + dx, y + dy) for dy in offsets for dx in offsets]
            alpha = sum(c[3] for c in samples)
            # Straight-alpha RGBA avoids a dark fringe on light Onion themes.
            rows.extend([round(sum(c[i] * c[3] for c in samples) / alpha) if alpha else 0
                         for i in range(3)] + [round(alpha / 16)])
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', 64, 64, 8, 6, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(bytes(rows), 9)) + chunk(b'IEND', b''))


def validate_archive(path):
    expected = set(FILES) | {'App/GrainMM/icon.png', 'App/GrainMM/MANIFEST.json'}
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)) or set(names) != expected:
            raise ValueError('package has duplicate, missing or unapproved members')
        for info in archive.infolist():
            if info.file_size > 2 * 1024 * 1024 or (info.external_attr >> 16) & 0o170000 != 0o100000:
                raise ValueError('package has oversized or non-regular members')
        manifest = json.loads(archive.read('App/GrainMM/MANIFEST.json'))
        if set(manifest['sha256']) != expected - {'App/GrainMM/MANIFEST.json'}:
            raise ValueError('manifest does not cover payload exactly')
        if manifest.get('status') == 'RELEASE CANDIDATE':
            validate_version(manifest.get('version', ''))
        elif manifest.get('status') != 'LOCAL EXPERIMENT - NOT A RELEASE':
            raise ValueError('invalid package status')
        for name, digest in manifest['sha256'].items():
            info = archive.getinfo(name)
            if info.file_size > 2 * 1024 * 1024:
                raise ValueError('unexpected payload size')
            if hashlib.sha256(archive.read(name)).hexdigest() != digest:
                raise ValueError(f'checksum mismatch: {name}')
        if archive.testzip() is not None:
            raise ValueError('corrupt ZIP')


def validate_version(version):
    if not isinstance(version, str) or not re.fullmatch(
            r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(?:-(?:alpha|beta|rc)\.[1-9][0-9]*)?', version):
        raise ValueError('invalid release version')


def release_version(version):
    validate_version(version)
    metadata = json.loads((ROOT / 'packaging/release.json').read_text())
    if metadata['version'] != version:
        raise ValueError('tag/version does not match packaging/release.json')
    if '-' not in version and metadata.get('stable_ready') is not True:
        raise ValueError('stable acceptance is incomplete; use the documented prerelease version')
    return version


def install_markdown(text, source, version):
    """Keep installed guides/license links usable after their files are flattened."""
    destinations = {value: Path(name).name for name, value in FILES.items()}
    def link(match):
        target = match[1]
        if re.match(r'^[a-zA-Z][a-zA-Z0-9+.-]*:', target) or target.startswith('#'):
            return match[0]
        name, sep, fragment = target.partition('#')
        path = (ROOT / source).parent.joinpath(name).resolve()
        if not path.is_relative_to(ROOT):
            raise ValueError('unsafe installation guide link')
        relative = path.relative_to(ROOT).as_posix()
        destination = destinations.get(relative)
        if destination is None:
            kind = 'tree' if path.is_dir() else 'blob'
            destination = f'https://github.com/lkotlarenko/GrainMM/{kind}/v{version}/{relative}'
        return '](' + destination + (sep + fragment if sep else '') + ')'
    return re.sub(r'\]\(([^)]+)\)', link, text)


def build(output, version=None):
    if version is not None:
        release_version(version)
    spec = spec_from_file_location('validate_elf', ROOT / 'scripts/toolchain/validate_elf.py')
    validator = module_from_spec(spec)
    spec.loader.exec_module(validator)
    validator.validate(ROOT / FILES['App/GrainMM/grainmm'])
    payload = {}
    for target, source in FILES.items():
        path = ROOT / source
        if path.is_symlink() or not path.is_file():
            raise ValueError(f'unsafe or missing package source: {source}')
        data = path.read_bytes()
        if path.suffix == '.md':
            data = install_markdown(data.decode(), source, version or json.loads(
                (ROOT / 'packaging/release.json').read_text())['version']).encode()
        payload[target] = data
    metadata = json.loads((ROOT / 'packaging/release.json').read_text())
    notice_name = 'App/GrainMM/SOURCE_NOTICE.txt'
    notice = payload[notice_name].decode().replace('<version>', version or metadata['version'])
    distribution = ('Matching source/build materials are distributed with this release under GPLv3.'
                    if metadata.get('source_publication_authorized', False) else
                    'Local experiment: retain the matching checkout; public distribution is not authorized.')
    notice = notice.replace('<distribution_notice>', distribution)
    payload[notice_name] = notice.encode()
    payload['App/GrainMM/icon.png'] = icon()
    payload['App/GrainMM/MANIFEST.json'] = (json.dumps({
        'status': 'RELEASE CANDIDATE' if version else 'LOCAL EXPERIMENT - NOT A RELEASE',
        'version': version,
        'sha256': {name: hashlib.sha256(data).hexdigest() for name, data in payload.items()},
    }, indent=2) + '\n').encode()
    # Validate in memory before creating an output. Never overwrite an existing archive.
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, 'w', zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(payload.items()):
            member = zipfile.ZipInfo(name, (2026, 1, 1, 0, 0, 0))
            mode = 0o755 if name.endswith(('.sh', '/grainmm')) else 0o644
            member.create_system = 3
            member.external_attr = (0o100000 | mode) << 16
            member.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(member, data)
    validate_archive(io.BytesIO(buffer.getvalue()))
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open('xb') as stream:
        stream.write(buffer.getvalue())
    print(f'Validated package: {output} ({output.stat().st_size} bytes)')
    if version:
        checksum = output.with_suffix(output.suffix + '.sha256')
        with checksum.open('x') as stream:
            stream.write(hashlib.sha256(buffer.getvalue()).hexdigest() + '  ' + output.name + '\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--local-experiment', action='store_true')
    mode.add_argument('--release-version')
    parser.add_argument('--output', type=Path, default=None)
    parser.add_argument('--validate', type=Path)
    args = parser.parse_args()
    try:
        if args.validate:
            validate_archive(args.validate)
            print('Package allowlist, checksums and member integrity passed')
        elif args.release_version:
            version = release_version(args.release_version)
            output = args.output or ROOT / f'dist/GrainMM-{version}-miyoo-mini-plus.zip'
            build(output, version)
        elif not args.local_experiment:
            sys.exit('Choose --local-experiment or --release-version; publication is a separate workflow')
        else:
            build(args.output or ROOT / 'dist/GrainMM-local-proof.zip')
    except (OSError, ValueError, zipfile.BadZipFile, KeyError) as error:
        sys.exit(f'Package rejected: {error}')
