#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Release asset regressions: checksums alone cannot prove matching source."""
import hashlib
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest
import zipfile
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts/packaging'))
import package
import source_archive
from verify_release import verify
VERSION = '1.0.0-alpha.1'


class Releases(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        data = {'packaging/release.json': json.dumps({'version': VERSION}),
                'LICENSE_SCOPE.md': 'scope', 'LICENSES/GPL-3.0-only.txt': 'license',
                'src/original.cpp': '// independently authored test fixture'}
        for name, text in data.items():
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)
        (self.root / 'packaging/source-files.txt').write_text('\n'.join(data) + '\n')
        self.source = self.root / f'GrainMM-{VERSION}-source.tar.gz'
        source_archive.build_source(self.source, VERSION, self.root)
        self.install = self.root / f'GrainMM-{VERSION}-miyoo-mini-plus.zip'
        payload = {name: b'test fixture' for name in package.FILES}
        payload['App/GrainMM/icon.png'] = package.icon()
        manifest = {'status': 'RELEASE CANDIDATE', 'version': VERSION,
                    'sha256': {name: hashlib.sha256(value).hexdigest() for name, value in payload.items()}}
        payload['App/GrainMM/MANIFEST.json'] = json.dumps(manifest).encode()
        with zipfile.ZipFile(self.install, 'w') as archive:
            for name, value in payload.items():
                member = zipfile.ZipInfo(name)
                member.create_system = 3
                member.external_attr = 0o100644 << 16
                archive.writestr(member, value)
        self.checksum(self.install)

    def checksum(self, path):
        path.with_suffix(path.suffix + '.sha256').write_text(
            hashlib.sha256(path.read_bytes()).hexdigest() + '  ' + path.name + '\n')

    def test_exact_checkout_passes(self):
        verify(self.root, VERSION, self.root)

    def test_asset_corruption(self):
        self.install.write_bytes(self.install.read_bytes() + b'changed')
        with self.assertRaisesRegex(ValueError, 'checksum'):
            verify(self.root, VERSION, self.root)

    def test_different_checkout_rejected(self):
        (self.root / 'src/original.cpp').write_text('// changed after packaging')
        with self.assertRaisesRegex(ValueError, 'checkout'):
            verify(self.root, VERSION, self.root)

    def test_wrong_version_rejected(self):
        with self.assertRaisesRegex(ValueError, 'version'):
            verify(self.root, '1.0.0-alpha.2', self.root)

    def test_checksum_cannot_substitute_unsafe_source(self):
        with tarfile.open(self.source, 'r:gz') as archive:
            entries = [(m, archive.extractfile(m).read()) for m in archive.getmembers()]
        member = tarfile.TarInfo('../escape')
        member.size = 3
        entries.append((member, b'bad'))
        with tarfile.open(self.source, 'w:gz') as archive:
            for member, content in entries:
                archive.addfile(member, io.BytesIO(content))
        self.checksum(self.source)
        with self.assertRaisesRegex(ValueError, 'paths'):
            verify(self.root, VERSION, self.root)

    def test_asset_symlink_rejected(self):
        original = self.install.with_suffix('.backup')
        self.install.rename(original)
        self.install.symlink_to(original)
        with self.assertRaisesRegex(ValueError, 'regular file'):
            verify(self.root, VERSION, self.root)


if __name__ == '__main__':
    unittest.main()
