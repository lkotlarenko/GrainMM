#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# SD-card app only. No governor, volume, MainUI, firmware or rootfs modifications.
set -eu
app_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$app_dir"
# Development state stays inside the explicitly approved application directory.
state_dir="$app_dir/state"
umask 077
if [ -L "$state_dir" ]; then echo 'GrainMM: refusing symlink state directory' >&2; exit 1; fi
binary="$app_dir/grainmm"
if [ -f "$app_dir/CURRENT" ]; then
    release=$(cat "$app_dir/CURRENT")
    case "$release" in ''|*[!a-f0-9]*) echo 'GrainMM: invalid release selector' >&2; exit 1;; esac
    if [ "${#release}" -ne 12 ]; then echo 'GrainMM: invalid release length' >&2; exit 1; fi
    binary="$app_dir/releases/$release/grainmm"
fi
mkdir -p "$state_dir"
export GRAINMM_STATE_DIR="$state_dir"
export GRAINMM_SAMPLE_DIR="$state_dir"
export GRAINMM_RESUME_SAMPLE=1
# Fixed two-file rotation; do not follow a symlink log.
for path in "$state_dir/session.log" "$state_dir/session.previous.log"; do
    if [ -L "$path" ]; then echo 'GrainMM: refusing symlink log' >&2; exit 1; fi
    if [ -e "$path" ] && [ ! -f "$path" ]; then echo 'GrainMM: log is not a regular file' >&2; exit 1; fi
done
if [ -f "$state_dir/session.log" ]; then
    mv "$state_dir/session.log" "$state_dir/session.previous.log"
fi
if [ -e "$state_dir/sample.wav" ] || [ -L "$state_dir/sample.wav" ]; then
    exec "$binary" --sample-dir "$state_dir" --sample sample.wav >"$state_dir/session.log" 2>&1
fi
exec "$binary" >"$state_dir/session.log" 2>&1
