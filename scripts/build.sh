#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
profile=${1:-host-debug}
case "$profile" in
    host-debug|host-asan-ubsan|miyoo-armv7-release) ;;
    *) echo 'Unknown build profile' >&2; exit 1 ;;
esac
cmake --preset "$profile"
cmake --build --preset "$profile"
case "$profile" in
    host-*) ctest --preset "$profile" ;;
    miyoo-*)
        for binary in grainmm grainmm-render grainmm-export granular_benchmark instrument_benchmark equalizer_benchmark; do
            python3 scripts/toolchain/validate_elf.py "build/$profile/$binary"
        done
        ;;
esac
