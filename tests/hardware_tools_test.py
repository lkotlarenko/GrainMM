#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Host-only RFB fixtures and deployment/rollback tests; never contacts a device."""
import hashlib
import json
import sys
import importlib.util
import io
from pathlib import Path
import struct
import subprocess
import tarfile
import tempfile
import unittest
from unittest.mock import patch
import zlib

ROOT = Path(__file__).resolve().parents[1]


def module(name, source):
    spec = importlib.util.spec_from_file_location(name, ROOT / source)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


deploy = module('deploy', 'scripts/miyoo/deploy.py')
vnc = module('vnc', 'scripts/miyoo/vnc_capture.py')
with patch.dict(sys.modules, {'deploy': deploy}):
    remote = module('remote', 'scripts/miyoo/remote.py')


class Socket:
    def __init__(self, data):
        self.data, self.sent = data, bytearray()

    def recv(self, size):
        result = self.data[:min(size, 7)]
        self.data = self.data[len(result):]
        return result

    def sendall(self, data):
        self.sent.extend(data)


def server_frame(encoding=0, width=2):
    init = struct.pack('>HH', 2, 1) + bytes(16) + struct.pack('>I', 4) + b'test'
    frame = b'\0\0\0\1' + struct.pack('>HHHHi', 0, 0, width, 1, encoding)
    return b'RFB 003.008\n\1\1' + bytes(4) + init + frame + b'\0\0\xff\0\0\xff\0\0'


def archive(binary=b'first', bootstrap=b'#!/bin/sh\nexit 0\n', tamper=False):
    payload = {'grainmm': binary, 'launch.sh': bootstrap, 'config.json': b'{}', 'icon.png': b'original fixture'}
    payload['SHA256SUMS'] = ''.join(f'{hashlib.sha256(data).hexdigest()}  {name}\n'
                                  for name, data in payload.items()).encode()
    if tamper:
        payload['grainmm'] = b'corrupt'
    buffer = io.BytesIO()
    with tarfile.open(fileobj=buffer, mode='w') as stream:
        for name, data in payload.items():
            info = tarfile.TarInfo(name)
            info.size = len(data)
            stream.addfile(info, io.BytesIO(data))
    return buffer.getvalue()


class HardwareTools(unittest.TestCase):
    def test_large_remote_script_streamed_over_stdin(self):
        # Embedded SSH servers may reject large exec requests. Run a real local
        # shell with a long quoted script to verify stdin transport and semantics.
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            config = root / 'connection.json'
            config.write_text('{}')
            script = root / 'probe.sh'
            output = root / 'evidence.txt'
            script.write_text('# comment\n' * 4096 +
                              "printf '%s' 'literal $payload' > " + str(output) + '\n')
            argv = ['remote.py', '--config', str(config), '--script', str(script)]
            with patch.object(sys, 'argv', argv), patch.object(remote, 'ssh_command', return_value=['sh', '-s']) as command:
                remote.main()
            self.assertEqual(command.call_args.args[1], 'exec sh -s')
            self.assertEqual(output.read_text(), 'literal $payload')

    def test_raw_capture_and_rgb(self):
        connection = Socket(server_frame())
        image, info = vnc.capture(connection)
        self.assertEqual((info['width'], info['height'], info['requested_frames']), (2, 1, 1))
        self.assertTrue(info['shared'])
        self.assertTrue(image.startswith(b'\x89PNG\r\n\x1a\n'))
        at = 8
        while at < len(image):
            size = struct.unpack('>I', image[at:at+4])[0]
            if image[at+4:at+8] == b'IDAT':
                self.assertEqual(zlib.decompress(image[at+8:at+8+size]), b'\0\xff\0\0\0\xff\0')
                break
            at += size+12
        else:
            self.fail('PNG missing image data')

    def test_capture_bounds_and_security(self):
        for data in (server_frame(encoding=7), server_frame(width=3), b'RFB 003.003\n',
                     b'RFB 003.008\n\1\2'):
            with self.assertRaises(ValueError):
                vnc.capture(Socket(data))
        with self.assertRaises(ConnectionError):
            vnc.capture(Socket(b''))

    def test_key_released(self):
        connection = Socket(server_frame())
        with patch.object(vnc.time, 'sleep'):
            vnc.capture(connection, 'Down')
        self.assertIn(struct.pack('>BBHI', 4, 1, 0, vnc.KEYS['Down']), connection.sent)
        self.assertIn(struct.pack('>BBHI', 4, 0, 0, vnc.KEYS['Down']), connection.sent)

    def test_input_without_frame_request(self):
        connection = Socket(server_frame())
        with patch.object(vnc.time, 'sleep'):
            image, info = vnc.capture(connection, 'Down', view=False)
        self.assertIsNone(image)
        self.assertEqual(info['requested_frames'], 0)
        self.assertNotIn(struct.pack('>BBHHHH', 3, 0, 0, 0, 2, 1), connection.sent)

    def run_install(self, root, release, payload):
        return subprocess.run(['sh', '-c', deploy.install_script(release, str(root), processes=False)],
                              input=payload, capture_output=True)

    def test_install_upgrade_and_rollback_preserve_state(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / 'GrainMM'
            first, second = 'a'*12, 'b'*12
            self.assertEqual(self.run_install(root, first, archive()).returncode, 0)
            (root / 'LAST_GOOD').write_text(first+'\n')
            (root / 'state').mkdir()
            sample = root / 'state/sample.wav'
            sample.write_bytes(b'user sample')
            self.assertEqual(self.run_install(root, second, archive(b'second')).returncode, 0)
            self.assertEqual((root / 'CURRENT').read_text().strip(), second)
            self.assertEqual((root / 'PREVIOUS').read_text().strip(), first)
            result = subprocess.run(['sh', '-c', deploy.rollback_script(str(root), processes=False)],
                                    capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual((root / 'CURRENT').read_text().strip(), first)
            self.assertEqual((root / f'releases/{second}/grainmm').read_bytes(), b'second')
            self.assertEqual(sample.read_bytes(), b'user sample')
            self.assertNotEqual(self.run_install(root, first, archive()).returncode, 0)

    def test_foreign_directory_and_symlink_refused(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / 'GrainMM'
            root.mkdir()
            protected = root / 'personal'
            protected.write_bytes(b'preserve')
            self.assertNotEqual(self.run_install(root, 'a'*12, archive()).returncode, 0)
            self.assertEqual(list(root.iterdir()), [protected])
            link = Path(temporary) / 'link'
            link.symlink_to(root)
            self.assertNotEqual(self.run_install(link, 'a'*12, archive()).returncode, 0)

    def test_failed_hash_and_bootstrap_keep_selector(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / 'GrainMM'
            self.assertEqual(self.run_install(root, 'a'*12, archive()).returncode, 0)
            self.assertNotEqual(self.run_install(root, 'b'*12, archive(tamper=True)).returncode, 0)
            self.assertEqual((root / 'CURRENT').read_text().strip(), 'a'*12)
            self.assertNotEqual(self.run_install(root, 'c'*12, archive(bootstrap=b'changed')).returncode, 0)
            self.assertEqual((root / 'CURRENT').read_text().strip(), 'a'*12)


if __name__ == '__main__':
    unittest.main()
