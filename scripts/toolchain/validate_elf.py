#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Static ARM ABI gate. A pass is not proof of compatibility with physical MM+."""
import re
import struct
import subprocess
import sys
from pathlib import Path


def validate(path):
    data = Path(path).read_bytes()
    if len(data) < 52 or data[:6] != b'\x7fELF\x01\x01':
        raise ValueError('expected little-endian ELF32')
    if struct.unpack_from('<H', data, 18)[0] != 40:
        raise ValueError('expected ARM machine')
    if not struct.unpack_from('<I', data, 36)[0] & 0x400:
        raise ValueError('missing ELF hard-float ABI flag')
    report = subprocess.check_output(['readelf', '-l', '-d', '-A', '-V', str(path)], text=True)
    if '/lib/ld-linux-armhf.so.3' not in report:
        raise ValueError('unexpected dynamic loader')
    if re.search(r'\[(?:libGL|libEGL|libGLES)', report):
        raise ValueError('GL/GLES/EGL dependency forbidden')
    if '(RPATH)' in report or '(RUNPATH)' in report:
        raise ValueError('unreviewed runtime search path')
    for version in re.findall(r'\bGLIBC_([0-9.]+)', report):
        if tuple(map(int, version.split('.'))) > (2, 28):
            raise ValueError('GLIBC requirement exceeds pinned SDK 2.28')
    needed = re.findall(r'\(NEEDED\).*?\[(.*?)\]', report)
    allowed = {'libSDL-1.2.so.0', 'libstdc++.so.6', 'libm.so.6', 'libgcc_s.so.1', 'libc.so.6', 'libpthread.so.0'}
    if set(needed) - allowed:
        raise ValueError(f'unreviewed dependencies: {set(needed) - allowed}')
    if 'Tag_ABI_VFP_args: VFP registers' not in report:
        raise ValueError('missing VFP argument ABI attribute')
    print('ELF32 ARM hard-float; interpreter /lib/ld-linux-armhf.so.3')
    print('NEEDED:', ', '.join(needed))
    print('Symbol versions:', ', '.join(sorted(set(re.findall(r'(?:GLIBCXX|GLIBC|CXXABI)_[0-9.]+', report)))))
    if 'Tag_Advanced_SIMD_arch: NEON' in report:
        print('NOTE: linked SDK inputs declare NEON; verify device Features before execution.')
    print('Static checks only; device library versions and transitive dependencies still require inspection.')


if __name__ == '__main__':
    try:
        validate(sys.argv[1])
    except (IndexError, ValueError, OSError, subprocess.CalledProcessError) as error:
        sys.exit(f'ELF validation failed: {error}')
