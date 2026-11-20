#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Adversarial archive gates and SD-only launcher checks using temporary original fixtures."""
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location('package', ROOT / 'scripts/packaging/package.py')
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


def fixture(extra=None, tamper=False, symlink=False):
    payload = {name: b'original test fixture' for name in package.FILES}
    payload['App/GrainMM/icon.png'] = package.icon()
    manifest = {'status': 'LOCAL EXPERIMENT - NOT A RELEASE', 'sha256': {
        name: hashlib.sha256(data).hexdigest() for name, data in payload.items()}}
    payload['App/GrainMM/MANIFEST.json'] = json.dumps(manifest).encode()
    if extra:
        payload[extra] = b'must not pass'
    if tamper:
        payload['App/GrainMM/grainmm'] = b'corruption'
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, 'w') as archive:
        for name, data in payload.items():
            member = zipfile.ZipInfo(name)
            member.external_attr = (0o120777 if symlink else 0o100644) << 16
            archive.writestr(member, data)
    return io.BytesIO(buffer.getvalue())


class Gates(unittest.TestCase):
    def test_valid(self):
        package.validate_archive(fixture())

    def test_injected_payload(self):
        for name in ('App/GrainMM/main.py', '../escape', '/absolute', 'GrainMM_Codex_Master_Prompt.md'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                package.validate_archive(fixture(extra=name))

    def test_corrupt_payload(self):
        with self.assertRaises(ValueError):
            package.validate_archive(fixture(tamper=True))

    def test_symlink_payload(self):
        with self.assertRaises(ValueError):
            package.validate_archive(fixture(symlink=True))

    def test_explicit_package_mode_required(self):
        result = subprocess.run(['python3', str(ROOT / 'scripts/packaging/package.py')], capture_output=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn(b'Choose --local-experiment', result.stderr)

    def test_release_version_gate(self):
        version = json.loads((ROOT / 'packaging/release.json').read_text())['version']
        self.assertEqual(package.release_version(version), version)
        for invalid in ('../../escape', 'v0.1.0', '0.1.0;echo bad', '00.1.0', '0.1.0-alpha.0'):
            with self.subTest(version=invalid), self.assertRaises(ValueError):
                package.validate_version(invalid)
        with self.assertRaises(ValueError):
            package.release_version('0.1.0')

    def test_launcher_rotation_and_symlinks(self):
        with tempfile.TemporaryDirectory() as temporary:
            sd = Path(temporary)
            app = sd / 'App/GrainMM'
            app.mkdir(parents=True)
            shutil.copyfile(ROOT / 'packaging/onion/launch.sh', app / 'launch.sh')
            executable = app / 'grainmm'
            executable.write_text('#!/bin/sh\nprintf "test-session\\n"\nfor arg; do printf "%s\\n" "$arg"; done\n')
            executable.chmod(0o755)
            executable.write_text(executable.read_text() +
                                  'printf "%s\\n%s\\n" "$GRAINMM_SAMPLE_DIR" "$GRAINMM_RESUME_SAMPLE" > "$GRAINMM_STATE_DIR/launch-environment"\n')
            for _ in range(2):
                subprocess.run(['sh', str(app / 'launch.sh')], check=True)
            state = app / 'state'
            self.assertEqual((state / 'session.log').read_text(), 'test-session\n')
            self.assertEqual((state / 'launch-environment').read_text().splitlines(), [str(state), '1'])
            self.assertEqual((state / 'session.previous.log').read_text(), 'test-session\n')
            sample = state / 'sample.wav'
            sample.write_bytes(b'preserve user sample')
            subprocess.run(['sh', str(app / 'launch.sh')], check=True)
            self.assertIn('--sample\nsample.wav\n', (state / 'session.log').read_text())
            self.assertEqual(sample.read_bytes(), b'preserve user sample')
            (state / 'session.log').unlink()
            protected = sd / 'protected'
            protected.write_text('preserve')
            (state / 'session.log').symlink_to(protected)
            result = subprocess.run(['sh', str(app / 'launch.sh')], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(protected.read_text(), 'preserve')

    def test_installed_guide_links(self):
        text = ('[GPL](../LICENSES/GPL-3.0-only.txt) '
                '[sound](PARAMETERS.md#samples) [start](../README.md#your-first-sound) '
                '[local](#first-installation)')
        rendered = package.install_markdown(text, 'docs/INSTALLATION.md', '1.0.0-alpha.1')
        self.assertIn('(GPL-3.0-only.txt)', rendered)
        self.assertIn('(USER_GUIDE.md#samples)', rendered)
        self.assertIn('https://github.com/lkotlarenko/GrainMM/blob/v1.0.0-alpha.1/README.md#your-first-sound', rendered)
        self.assertIn('( #first-installation)'.replace(' ', ''), rendered)

    def test_wrong_elf(self):
        result = subprocess.run(['python3', str(ROOT / 'scripts/toolchain/validate_elf.py'), '/bin/sh'],
                                capture_output=True)
        self.assertNotEqual(result.returncode, 0)


if __name__ == '__main__':
    unittest.main()
