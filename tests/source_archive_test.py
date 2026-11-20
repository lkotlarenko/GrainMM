#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
import tarfile
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('source_archive', ROOT / 'scripts/packaging/source_archive.py')
source = importlib.util.module_from_spec(spec)
spec.loader.exec_module(source)


class SourceTests(unittest.TestCase):
    def fixture(self, folder):
        root = Path(folder)
        for path, data in {'packaging/release.json': json.dumps({'version': '0.1.0-alpha.10'}),
                           'LICENSE_SCOPE.md': 'original scope', 'LICENSES/GPL-3.0-only.txt': 'license fixture',
                           'src/original.cpp': '// original fixture'}.items():
            p = root / path
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(data)
        (root / 'packaging/source-files.txt').write_text('LICENSE_SCOPE.md\nLICENSES/GPL-3.0-only.txt\nsrc/original.cpp\n')
        return root

    def test_repeatable_exact_inputs(self):
        with tempfile.TemporaryDirectory() as folder:
            root = self.fixture(folder)
            a, b = root / 'a.tar.gz', root / 'b.tar.gz'
            source.build_source(a, '0.1.0-alpha.10', root)
            source.build_source(b, '0.1.0-alpha.10', root)
            self.assertEqual(a.read_bytes(), b.read_bytes())
            with tarfile.open(fileobj=io.BytesIO(a.read_bytes())) as archive:
                names = archive.getnames()
                self.assertEqual(len(names), 4)
                self.assertTrue(any(n.endswith('/src/original.cpp') for n in names))
                self.assertTrue(all(m.isfile() for m in archive.getmembers()))
            with self.assertRaises(FileExistsError):
                source.build_source(a, '0.1.0-alpha.10', root)
            with self.assertRaises(ValueError):
                source.build_source(root / 'bad.tar.gz', '0.1.0-alpha.11', root)

    def test_private_paths_and_symlinks_rejected(self):
        for forbidden in ['.local/private.txt', '../escape', '/absolute', 'GrainMM_Codex_Master_Prompt.md', 'src/sound.wav']:
            with self.subTest(path=forbidden), tempfile.TemporaryDirectory() as folder:
                root = self.fixture(folder)
                with (root / 'packaging/source-files.txt').open('a') as stream:
                    stream.write(forbidden + '\n')
                with self.assertRaises(ValueError):
                    source.source_payload(root)
        with tempfile.TemporaryDirectory() as folder:
            root = self.fixture(folder)
            (root / 'src/original.cpp').unlink()
            (root / 'src/original.cpp').symlink_to(root / 'LICENSE_SCOPE.md')
            with self.assertRaises(ValueError):
                source.source_payload(root)


if __name__ == '__main__':
    unittest.main()
