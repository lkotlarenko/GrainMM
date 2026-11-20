#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Host-only SDL dummy backend: format sweeps, invalid startup and signal shutdown."""
import os
from pathlib import Path
import signal
import re
import subprocess
import time
import unittest

ROOT = Path(__file__).resolve().parent.parent
BINARY = ROOT / 'build/host-debug/grainmm'
ENV = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')


class Runtime(unittest.TestCase):
    def test_invalid_arguments(self):
        for args in (('--buffer', 'bad'), ('--seconds', '0'), ('--unknown',)):
            with self.subTest(args=args):
                result = subprocess.run([str(BINARY), *args], env=ENV, capture_output=True)
                self.assertNotEqual(result.returncode, 0)

    def test_help_does_not_initialize_audio(self):
        for action in ['--help','--version']:
            result=subprocess.run([str(BINARY),'--self-test-harmony',action],
                env=dict(ENV,SDL_AUDIODRIVER='grainmm-missing-driver'),capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)

    def test_missing_audio(self):
        env = dict(ENV, SDL_AUDIODRIVER='grainmm-missing-driver')
        result = subprocess.run([str(BINARY), '--seconds', '1'], env=env, capture_output=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn(b'GrainMM:', result.stderr)

    def test_buffer_sweep_and_relaunch(self):
        for buffer in (256, 512, 1024, 1024, 1024):
            with self.subTest(buffer=buffer):
                result = subprocess.run([str(BINARY), '--self-test', '--seconds', '1',
                                         '--buffer', str(buffer)], env=ENV, capture_output=True,
                                        text=True, timeout=5)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn('toggles=1', result.stdout)
                self.assertIn(f'frames={buffer}', result.stdout)
                self.assertIn(f'callback_min={buffer} callback_max={buffer}', result.stdout)
                callbacks = int(re.search(r'exit callbacks=(\d+)', result.stdout).group(1))
                processed = int(re.search(r'audio_frames=(\d+)', result.stdout).group(1))
                self.assertEqual(processed, callbacks * buffer)

    def test_signal_exit(self):
        process = subprocess.Popen([str(BINARY), '--seconds', '10'], env=ENV,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            time.sleep(0.3)
            process.send_signal(signal.SIGTERM)
            stdout, stderr = process.communicate(timeout=3)
            self.assertEqual(process.returncode, 0, stderr)
            self.assertIn(b'exit callbacks=', stdout)
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()


if __name__ == '__main__':
    unittest.main()
