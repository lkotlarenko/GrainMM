#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Prepare immutable GrainMM releases; default is read-only remote preflight."""
import argparse
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import re
import shlex
import subprocess
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]
TARGET = '/mnt/SDCARD/App/GrainMM'
VERSION = 'GrainMM-v1'


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def prepare(package):
    codec = load_module('grainmm_package', ROOT / 'scripts/packaging/package.py')
    codec.validate_archive(package)
    with zipfile.ZipFile(package) as archive:
        payload = {name.removeprefix('App/GrainMM/'): archive.read(name)
                   for name in archive.namelist()}
    validator = load_module('grainmm_elf', ROOT / 'scripts/toolchain/validate_elf.py')
    application = ROOT / 'build/miyoo-armv7-release/grainmm'
    validator.validate(application)
    if payload['grainmm'] != application.read_bytes():
        raise ValueError('package does not match the currently reviewed ARM app build')
    for binary in ('granular_benchmark', 'grainmm-render', 'instrument_benchmark'):
        path = ROOT / 'build/miyoo-armv7-release' / binary
        validator.validate(path)
        payload[binary] = path.read_bytes()
    digests = {name: hashlib.sha256(data).hexdigest() for name, data in sorted(payload.items())}
    release = hashlib.sha256(json.dumps(digests, sort_keys=True).encode()).hexdigest()[:12]
    payload['SHA256SUMS'] = ''.join(f'{digest}  {name}\n' for name, digest in digests.items()).encode()
    buffer = io.BytesIO()
    with tarfile.open(fileobj=buffer, mode='w') as archive:
        for name, data in sorted(payload.items()):
            if not re.fullmatch(r'[A-Za-z0-9_.-]+', name):
                raise ValueError('deployment requires flat reviewed filenames')
            info = tarfile.TarInfo(name)
            info.mode = 0o755 if name in ('grainmm', 'granular_benchmark', 'grainmm-render', 'instrument_benchmark') or name.endswith('.sh') else 0o644
            info.size = len(data)
            archive.addfile(info, io.BytesIO(data))
    return release, buffer.getvalue(), digests


def preflight(root=TARGET, processes=True):
    script = f'''set -eu
root={shlex.quote(root)}
if [ -L "$root" ] || [ -L "$root/releases" ] || [ -L "$root/state" ]; then
    echo 'Refusing symlink development directories' >&2; exit 1
fi
if [ -e "$root" ]; then
    [ -d "$root" ] || exit 1
    [ -f "$root/DEPLOYMENT_VERSION" ] || exit 1
    [ ! -L "$root/DEPLOYMENT_VERSION" ] || exit 1
    [ "$(cat "$root/DEPLOYMENT_VERSION")" = '{VERSION}' ] || exit 1
    for name in CURRENT PREVIOUS LAST_GOOD launch.sh config.json icon.png; do
        [ ! -L "$root/$name" ] || exit 1
        if [ -e "$root/$name" ]; then [ -f "$root/$name" ] || exit 1; fi
    done
fi
'''
    if processes:
        script += '''
for item in /proc/[0-9]*/exe; do
    exe=$(readlink "$item" 2>/dev/null || :)
    case "$exe" in "$root"/*) echo 'GrainMM is running; deployment refused' >&2; exit 1;; esac
done
'''
    return script


def install_script(release, root=TARGET, processes=True):
    if not re.fullmatch('[a-f0-9]{12}', release):
        raise ValueError('invalid release identity')
    return preflight(root, processes) + f'''
release='{release}'
if [ -e "$root/releases/$release" ] || [ -L "$root/releases/$release" ] ||
   [ -e "$root/.stage-$release" ] || [ -L "$root/.stage-$release" ]; then
    echo 'Release/staging collision refused' >&2; exit 1
fi
umask 077
mkdir -p "$root/releases"
if [ ! -f "$root/DEPLOYMENT_VERSION" ]; then printf '{VERSION}\\n' > "$root/DEPLOYMENT_VERSION"; fi
mkdir "$root/.stage-$release"
tar -xf - -C "$root/.stage-$release"
cd "$root/.stage-$release"
sha256sum -c SHA256SUMS
for name in launch.sh config.json icon.png; do
    if [ -e "$root/$name" ]; then
        cmp "$name" "$root/$name" || {{ echo 'Bootstrap migration needs separate review' >&2; exit 1; }}
    fi
done
mv "$root/.stage-$release" "$root/releases/$release"
for name in launch.sh config.json icon.png; do
    if [ ! -e "$root/$name" ]; then cp "$root/releases/$release/$name" "$root/$name"; fi
done
set -C
if [ -f "$root/CURRENT" ]; then
    previous=$(cat "$root/CURRENT")
    case "$previous" in ''|*[!a-f0-9]*) exit 1;; esac
    [ "${{#previous}}" -eq 12 ] || exit 1
    [ -d "$root/releases/$previous" ] || exit 1
    printf '%s\\n' "$previous" > "$root/PREVIOUS.next-$release"
    mv -f "$root/PREVIOUS.next-$release" "$root/PREVIOUS"
fi
printf '%s\\n' "$release" > "$root/CURRENT.next-$release"
mv -f "$root/CURRENT.next-$release" "$root/CURRENT"
printf 'Installed immutable release %s; no application launched\\n' "$release"
'''


def rollback_script(root=TARGET, processes=True, release=None):
    if release is not None and not re.fullmatch('[a-f0-9]{12}', release):
        raise ValueError('invalid rollback identity')
    selection = (f"previous='{release}'\n" if release else '''
[ -f "$root/LAST_GOOD" ] || { echo 'No verified rollback target recorded' >&2; exit 1; }
previous=$(cat "$root/LAST_GOOD")
''')
    return preflight(root, processes) + selection + '''
current=$(cat "$root/CURRENT")
[ "$previous" != "$current" ] || { echo 'Already on LAST_GOOD; choose a reviewed older release explicitly' >&2; exit 1; }
for release in "$previous" "$current"; do
    case "$release" in ''|*[!a-f0-9]*) exit 1;; esac
    [ "${#release}" -eq 12 ] || exit 1
    [ -d "$root/releases/$release" ] || exit 1
done
set -C
printf '%s\\n' "$previous" > "$root/CURRENT.rollback-$$"
printf '%s\\n' "$current" > "$root/PREVIOUS.rollback-$$"
mv -f "$root/CURRENT.rollback-$$" "$root/CURRENT"
mv -f "$root/PREVIOUS.rollback-$$" "$root/PREVIOUS"
echo 'GrainMM selector rolled back; both immutable builds retained'
'''


def ssh_command(config, script):
    user, host = config['user'], config['host']
    if not re.fullmatch(r'[A-Za-z0-9_.-]+', user) or not re.fullmatch(r'[A-Za-z0-9.:-]+', host):
        raise ValueError('invalid local SSH endpoint configuration')
    return ['ssh', '-S', config['control_socket'], '-o', 'BatchMode=yes', '-o', 'ConnectTimeout=5',
            '-o', 'StrictHostKeyChecking=yes', '-o', 'UserKnownHostsFile='+config['known_hosts'],
            '-p', str(int(config.get('port', 22))), user+'@'+host, 'sh -c '+shlex.quote(script)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True, help='ignored host-only connection JSON')
    parser.add_argument('--package', type=Path)
    parser.add_argument('--plan-dir', type=Path, default=ROOT / '.local/hardware/deployment')
    parser.add_argument('--apply', action='store_true', help='requires recorded explicit user approval')
    parser.add_argument('--rollback', action='store_true', help='requires recorded explicit user approval')
    parser.add_argument('--rollback-release', help='explicit reviewed immutable rollback identity')
    args = parser.parse_args()
    config = json.loads(args.config.read_text())
    plan_dir = args.plan_dir.resolve()
    if not plan_dir.is_relative_to(ROOT / '.local'):
        parser.error('private plans must remain under .local')
    plan_dir.mkdir(parents=True, exist_ok=True)
    if args.apply or args.rollback:
        approval = json.loads((plan_dir / 'approval.json').read_text())
        if approval.get('directory') != TARGET or approval.get('authorized') is not True:
            parser.error('explicit user approval for this development directory is required')
    if args.rollback:
        subprocess.run(ssh_command(config, rollback_script(release=args.rollback_release)), check=True, timeout=20)
        return
    if not args.package:
        parser.error('--package is required for preparation/deployment')
    release, archive, digests = prepare(args.package)
    plan = {'directory': TARGET, 'release': release, 'archive_bytes': len(archive), 'sha256': digests,
            'launches_application': False, 'writes_outside_directory': False}
    (plan_dir / 'plan.json').write_text(json.dumps(plan, indent=2)+'\n')
    (plan_dir / (release+'.tar')).write_bytes(archive)
    probe = preflight() + '''
available=$(df -Pk /mnt/SDCARD/App | awk 'END {print $4}')
[ "$available" -ge 2048 ] || { echo 'At least 2MiB free SD space required' >&2; exit 1; }
echo 'Read-only preflight passed; no remote files modified'
'''
    subprocess.run(ssh_command(config, probe), check=True, timeout=20)
    print(json.dumps(plan, sort_keys=True))
    if args.apply:
        subprocess.run(ssh_command(config, install_script(release)), input=archive, check=True, timeout=30)
    else:
        print('DRY RUN: host staging only; first deployment awaits approval')


if __name__ == '__main__':
    try:
        main()
    except subprocess.TimeoutExpired:
        raise SystemExit('Remote operation timed out. Inspect the existing SSH session before retrying.')
    except subprocess.CalledProcessError as error:
        raise SystemExit(f'Remote operation failed with status {error.returncode}; current release was not assumed changed.')
