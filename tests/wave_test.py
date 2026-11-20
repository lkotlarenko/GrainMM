#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Exercise the real decoder/writer and optional live adapter with synthetic WAVs only."""
import argparse
import math
import os
import re
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
import wave

parser = argparse.ArgumentParser()
parser.add_argument('--renderer', type=Path, required=True)
parser.add_argument('--player', type=Path)
parser.add_argument('--fault-library', type=Path)
args, remaining = parser.parse_known_args()
sys.argv = [sys.argv[0], *remaining]
if args.fault_library:
    os.environ['LD_PRELOAD'] = str(args.fault_library.resolve())


def chunk(name, data):
    return name + struct.pack('<I', len(data)) + data + (b'\0' if len(data) & 1 else b'')


def riff(chunks):
    payload = b'WAVE' + b''.join(chunks)
    return b'RIFF' + struct.pack('<I', len(payload)) + payload


def fmt(channels=1, rate=48000, bits=16, code=1, alignment=None, byte_rate=None):
    align = channels * bits // 8 if alignment is None else alignment
    return chunk(b'fmt ', struct.pack('<HHIIHH', code, channels, rate,
                                      rate * align if byte_rate is None else byte_rate, align, bits))


class WaveTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def render(self, name='input.wav', output='output.wav', directory=None):
        return subprocess.run([str(args.renderer), '--sample-dir', str(directory or self.root),
                               '--sample', name, '--output', str(self.root / output), '--seconds', '1'],
                              capture_output=True, text=True, timeout=10)

    def reject(self, payload):
        (self.root / 'input.wav').write_bytes(payload)
        result = self.render()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertFalse((self.root / 'output.wav').exists())

    def test_roundtrip_and_live_synthetic_audio(self):
        values = [int(18000 * math.sin(i * 2 * math.pi * 440 / 44100)) for i in range(4410)]
        stereo = b''.join(struct.pack('<hh', value, -value) for value in values)
        (self.root / 'input.wav').write_bytes(riff([fmt(2, 44100), chunk(b'data', stereo)]))
        result = self.render()
        self.assertEqual(result.returncode, 0, result.stderr)
        with wave.open(str(self.root / 'output.wav'), 'rb') as output:
            self.assertEqual((output.getnchannels(), output.getsampwidth(), output.getframerate(),
                              output.getnframes()), (2, 2, 48000, 48000))
            pcm = struct.unpack('<96000h', output.readframes(48000))
        self.assertGreater(max(pcm), 100)
        self.assertLessEqual(max(abs(value) for value in pcm), 16384)
        self.assertEqual(pcm[-2:], (0, 0))
        self.assertTrue(all(pcm[i] == -pcm[i+1] for i in range(0, len(pcm), 2)))
        self.assertEqual(sum(pcm), 0)
        if args.player:
            env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
            live = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--sample',
                                   'input.wav', '--self-test', '--seconds', '1'], env=env,
                                  capture_output=True, text=True, timeout=10)
            self.assertEqual(live.returncode, 0, live.stderr)
            self.assertIn('mode=granular', live.stdout)
            self.assertRegex(live.stdout, r'grains=[1-9][0-9]*')
            self.assertRegex(live.stdout, r'updates=[1-9][0-9]*')

    def test_temporary_snapshot_preserves_transport_and_rejects_changed_source(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        result = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--sample',
                                 'input.wav', '--state-dir', str(self.root), '--self-test-snapshot',
                                 '--seconds', '4'], env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertIn('snapshot_captures=2 snapshot_recalls=2 snapshot_rejections=2', result.stdout)
        self.assertIn('snapshot captures=1 recalls=1 playing=1', result.stdout)
        self.assertIn('snapshot captures=2 recalls=2 playing=0', result.stdout)
        self.assertIn('toggles=2', result.stdout)
        self.assertIn('help_views=0', result.stdout)
        self.assertIn('waveform_views=0', result.stdout)
        saved = (self.root / 'parameters.v18').read_text().splitlines()
        self.assertEqual(float(saved[1].split()[0]), 80)
        self.assertEqual(float(saved[1].split()[30]), 0)  # OSC/LEVEL recalled in built-in mode.
        self.assertEqual(saved[2], '1 ""')
        self.assertEqual(sorted(p.name for p in self.root.iterdir()), ['input.wav', 'parameters.v18'])

    def test_tools_export_navigation_and_reload(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000) * 4800)]))
        result = subprocess.run([str(args.player), '--sample-dir', str(self.root),
                                 '--sample', 'input.wav', '--state-dir', str(self.root),
                                 '--self-test-tools', '--seconds', '12'],
                                env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'),
                                capture_output=True, text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertIn('exports_completed=1 exports_cancelled=0 pattern_views=1', result.stdout)
        self.assertIn('source_changes=1', result.stdout)
        self.assertEqual((self.root / 'exports/GrainMM-001.wav').stat().st_size, 960044)
        self.assertIn('2 "exports/GrainMM-001.wav"', (self.root / 'parameters.v18').read_text())

    def test_manual_pattern_and_reopen(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000) * 4800)]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        base = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                '--state-dir', str(self.root)]
        result = subprocess.run(base + ['--self-test-pattern', '--seconds', '4'],
                                env=env, capture_output=True, text=True, timeout=12)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        saved = (self.root / 'parameters.v18').read_text()
        self.assertIn('Pattern 1 1', saved)
        step = saved.splitlines()[5].split()  # Pattern header, step 0, step 1.
        self.assertEqual(step[:3], ['1', '0', '1'])
        self.assertAlmostEqual(float(step[3]), .95, places=5)
        self.assertIn('toggles=2', result.stdout)
        reopened = subprocess.run(base + ['--seconds', '1'], env=env,
                                  capture_output=True, text=True, timeout=8)
        self.assertEqual(reopened.returncode, 0, reopened.stderr)
        self.assertIn('preset restored', reopened.stdout)
        self.assertEqual((self.root / 'parameters.v18').read_text(), saved)

    def test_live_parameter_save_and_reopen(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        command = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                   '--state-dir', str(self.root), '--self-test', '--seconds', '1']
        first = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertIn('preset saved', first.stdout)
        self.assertFalse((self.root / 'parameters.v18.pending').exists())
        saved = (self.root / 'parameters.v18').read_text().split()
        self.assertEqual(saved[:2], ['GrainMM', '18'])
        self.assertAlmostEqual(float(saved[10]), 0.05, places=5)
        second = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertIn('preset restored', second.stdout)
        self.assertIn('preset saved', second.stdout)
        self.assertAlmostEqual(float((self.root / 'parameters.v18').read_text().split()[10]), 0.1, places=5)
        (self.root / 'parameters.v18').write_text('unsupported user data')
        bad = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertNotEqual(bad.returncode, 0)
        self.assertEqual((self.root / 'parameters.v18').read_text(), 'unsupported user data')

    def test_live_browser_load_and_persistent_selection(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        command = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                   '--state-dir', str(self.root), '--self-test-browser', '--seconds', '2']
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('source_changes=1', result.stdout)
        self.assertIn('source switched: sample; paused', result.stdout)
        saved = (self.root / 'parameters.v18').read_text()
        self.assertIn('2 "input.wav"', saved)
        # No explicit sample argument: the real resume path must load the stored WAV.
        resumed = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--state-dir',
                                  str(self.root), '--resume-sample', '--self-test', '--seconds', '1'],
                                 env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(resumed.returncode, 0, resumed.stderr)
        self.assertIn('mode=granular', resumed.stdout)
        preserved = (self.root / 'parameters.v18').read_bytes()
        wave_bytes = (self.root / 'input.wav').read_bytes()
        (self.root / 'input.wav').unlink()
        missing = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--state-dir',
                                  str(self.root), '--resume-sample', '--seconds', '1'], env=env,
                                 capture_output=True, text=True, timeout=10)
        self.assertEqual(missing.returncode, 0, missing.stderr)
        self.assertIn('Sample restore failed:', missing.stderr)
        self.assertIn('mode=tone', missing.stdout)
        self.assertEqual((self.root / 'parameters.v18').read_bytes(), preserved)
        # New no-WAV controls remain editable during recovery. Saving those
        # edits must preserve the unavailable WAV selection until an explicit
        # browser choice replaces it, then the same file can return later.
        edited = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--state-dir',
                                 str(self.root), '--resume-sample', '--self-test', '--seconds', '1'],
                                env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(edited.returncode, 0, edited.stderr)
        self.assertIn('preset saved', edited.stdout)
        self.assertIn('source_mode=2 pending_restore=1', edited.stdout)
        saved_selection = (self.root / 'parameters.v18').read_bytes()
        self.assertIn(b'2 "input.wav"', saved_selection)
        self.assertNotEqual(saved_selection, preserved)
        (self.root / 'input.wav').write_bytes(wave_bytes)
        recovered = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--state-dir',
                                   str(self.root), '--resume-sample', '--seconds', '1'], env=env,
                                  capture_output=True, text=True, timeout=10)
        self.assertEqual(recovered.returncode, 0, recovered.stderr)
        self.assertIn('mode=granular', recovered.stdout)
        self.assertIn('source_mode=2 pending_restore=0', recovered.stdout)
        self.assertEqual((self.root / 'parameters.v18').read_bytes(), saved_selection)

    def test_live_browser_from_tone(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        # Keep state outside the library: directories sort before WAV files.
        with tempfile.TemporaryDirectory() as state:
            result = subprocess.run([str(args.player), '--sample-dir', str(self.root),
                                     '--state-dir', state, '--self-test-browser', '--seconds', '2'],
                                    env=env, capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('mode=tone', result.stdout)
            self.assertIn('source_changes=1', result.stdout)
            self.assertIn('source switched: sample; paused', result.stdout)
            self.assertRegex(result.stdout, r'grains=[1-9][0-9]*')
            self.assertIn('2 "input.wav"', (Path(state) / 'parameters.v18').read_text())

    def test_builtin_synth_without_sample(self):
        if not args.player:
            self.skipTest('player required')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        command = [str(args.player), '--sample-dir', str(self.root), '--state-dir', str(self.root),
                   '--resume-sample', '--self-test-synth', '--seconds', '2']
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('preset saved', result.stdout)
        saved = (self.root / 'parameters.v18').read_text().split()
        self.assertAlmostEqual(float(saved[32]), 0.01, places=5)
        self.assertAlmostEqual(float(saved[34]), 1, places=5)
        self.assertAlmostEqual(float(saved[43]), 0.05, places=5)
        self.assertAlmostEqual(float(saved[48]), 0.05, places=5)
        resumed = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(resumed.returncode, 0, resumed.stderr)
        self.assertIn('preset restored', resumed.stdout)
        self.assertAlmostEqual(float((self.root / 'parameters.v18').read_text().split()[32]), 0.02, places=5)

    def test_expanded_rack_and_help(self):
        if not args.player:
            self.skipTest('player required')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        command = [str(args.player), '--sample-dir', str(self.root), '--state-dir', str(self.root),
                   '--self-test-rack', '--seconds', '3']
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('help_views=1', result.stdout)
        fields = (self.root / 'parameters.v18').read_text().split()
        self.assertEqual(fields[:2], ['GrainMM', '18'])
        self.assertAlmostEqual(float(fields[55]), 0.05, places=5)
        self.assertAlmostEqual(float(fields[58]), 0.1, places=5)
        self.assertAlmostEqual(float(fields[59]), 0.1, places=5)
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        result = subprocess.run(command + ['--sample', 'input.wav'], env=env,
                                capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('help_views=1', result.stdout)
        fields = (self.root / 'parameters.v18').read_text().split()
        for field, expected in [(62, 0.01), (63, 0.99), (64, 0.22), (65, 0.22), (66, 0.1), (67, 0.9)]:
            self.assertAlmostEqual(float(fields[field]), expected, places=5)

    def test_texture_pages_and_guarded_effects_reset(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        base = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav', '--state-dir', str(self.root)]
        result = subprocess.run(base+['--self-test-texture','--seconds','4'],env=env,capture_output=True,text=True,timeout=12)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        data=(self.root/'parameters.v18').read_text().split()
        self.assertEqual(data[:2],['GrainMM','18'])
        for index,value in [(74,11),(75,2),(78,.05),(80,7900),(84,.05),(90,.05)]:
            self.assertAlmostEqual(float(data[index]),value,places=5)
        result = subprocess.run(base+['--self-test-fx-reset','--seconds','6'],env=env,capture_output=True,text=True,timeout=15)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('effects_resets=1',result.stdout)
        data=(self.root/'parameters.v18').read_text().split()
        self.assertEqual([float(data[i]) for i in [10,20,43,48,55,58,59,78,84,90]],[0]*10)
        self.assertEqual(float(data[74]),12)
        data[78]='0.3';data[84]='0.5'
        (self.root/'parameters.v18').write_text(' '.join(data)+'\n')
        result=subprocess.run(base+['--self-test-shortcut','--seconds','3'],env=env,capture_output=True,text=True,timeout=10)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('effects_resets=1',result.stdout)
        data=(self.root/'parameters.v18').read_text().split()
        self.assertEqual(float(data[78]),0)
        self.assertEqual(float(data[84]),0)
        self.assertIn('sound_reset scope=1 playing=1',result.stdout)
        result=subprocess.run(base+['--self-test-reset-sound','--seconds','3'],env=env,capture_output=True,text=True,timeout=10)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('sound_reset scope=2 playing=0',result.stdout)
        fields=(self.root/'parameters.v18').read_text().split()
        self.assertEqual(float(fields[2]),80)
        self.assertEqual(float(fields[4]),1)
        self.assertEqual(fields[153], '"input.wav"')


    def test_slice_controls_clock_playback_and_save(self):
        if not args.player:
            self.skipTest('player required')
        samples = struct.pack('<8000h', *[int(12000 * math.sin(i * .2)) for i in range(8000)])
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', samples)]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        base = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                '--state-dir', str(self.root)]
        result = subprocess.run(base + ['--self-test-slice', '--seconds', '4'], env=env,
                                capture_output=True, text=True, timeout=12)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertRegex(result.stdout, r'slices_triggered=[1-9][0-9]*')
        saved = (self.root / 'parameters.v18').read_bytes()
        fields = saved.decode().split()
        self.assertEqual(fields[:2], ['GrainMM', '18'])
        for index, expected in enumerate([1, 9, 3, .75, 1, .1], 92):
            self.assertAlmostEqual(float(fields[index]), expected, places=5)
        reopened = subprocess.run(base + ['--seconds', '1'], env=env, capture_output=True,
                                  text=True, timeout=10)
        self.assertEqual(reopened.returncode, 0, reopened.stderr)
        self.assertEqual((self.root / 'parameters.v18').read_bytes(), saved)

    def test_major_feature_banks_and_saved_controls(self):
        if not args.player:
            self.skipTest('player required')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        (self.root/'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<8000h', *([12000]*8000)))]))
        base = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav', '--state-dir', str(self.root)]
        result = subprocess.run(base + ['--self-test-performance', '--seconds', '9'],
                                env=env, capture_output=True, text=True, timeout=15)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertIn('toggles=2', result.stdout)
        values = list(map(float, (self.root/'parameters.v18').read_text().splitlines()[1].split()))
        expected = [.01, 58, 0, 0, .5, 0, .05, .1, .05, .05, .01, 2,
                    .05, 15, 1, 1, .05, 3]
        for actual, wanted in zip(values[132:], expected):
            self.assertAlmostEqual(actual, wanted, places=5)
        self.assertEqual(len(values), 150)
        saved = (self.root/'parameters.v18').read_bytes()
        reopened = subprocess.run(base + ['--seconds', '1'], env=env,
                                  capture_output=True, text=True, timeout=5)
        self.assertEqual(reopened.returncode, 0, reopened.stderr)
        self.assertEqual((self.root/'parameters.v18').read_bytes(), saved)

    def test_ambient_scenes_output_and_save(self):
        if not args.player:
            self.skipTest('player required')
        env = dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
        base=[str(args.player),'--sample-dir',str(self.root),'--state-dir',str(self.root)]
        result=subprocess.run(base+['--self-test-ambient','--seconds','12'],env=env,
                              capture_output=True,text=True,timeout=16)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('drone_scene_applies=1 drone_scene_cancels=2',result.stdout)
        self.assertIn('toggles=2',result.stdout)
        data=(self.root/'parameters.v18').read_text().splitlines()
        values=list(map(float,data[1].split()))
        self.assertEqual(len(values),150)
        self.assertAlmostEqual(values[114],.6,places=5)
        self.assertEqual(values[115:118],[67,4,8])
        self.assertEqual(values[124:126],[315,0])
        for actual, expected in zip(values[126:],[6,1,-2,210,.1,1]):
            self.assertAlmostEqual(actual,expected,places=5)
        self.assertIn('clipped_samples=0',result.stdout)
        self.assertEqual(data[2],'1 ""')
        saved=(self.root/'parameters.v18').read_bytes()
        reopened=subprocess.run(base+['--seconds','1'],env=env,capture_output=True,text=True,timeout=10)
        self.assertEqual(reopened.returncode,0,reopened.stderr)
        self.assertEqual((self.root/'parameters.v18').read_bytes(),saved)

    def test_standalone_drone_no_wave_shared_controls_and_save(self):
        if not args.player:
            self.skipTest('player required')
        env = dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
        base=[str(args.player),'--sample-dir',str(self.root),'--state-dir',str(self.root)]
        result=subprocess.run(base+['--self-test-drone','--seconds','8'],env=env,
                              capture_output=True,text=True,timeout=12)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('toggles=2',result.stdout)
        self.assertIn('help_views=1',result.stdout)
        self.assertRegex(result.stdout,r'drone_state=[1-9][0-9]*')
        data=(self.root/'parameters.v18').read_text().splitlines()
        values=list(map(float,data[1].split()))
        self.assertEqual(len(values),150)
        self.assertAlmostEqual(values[114],.25,places=5)
        self.assertEqual(values[115:118],[61,2,11])
        self.assertAlmostEqual(values[118],.4,places=5)
        self.assertAlmostEqual(values[119],.65,places=5)
        self.assertAlmostEqual(values[120],.30,places=5)
        self.assertAlmostEqual(values[121],.45,places=5)
        self.assertAlmostEqual(values[122],.55,places=5)
        self.assertAlmostEqual(values[123],.95,places=5)
        self.assertEqual(values[124:126],[43,1])
        self.assertRegex(result.stdout,r'drone_events=[1-9][0-9]*')
        self.assertEqual(data[2],'1 ""')
        saved=(self.root/'parameters.v18').read_bytes()
        reopened=subprocess.run(base+['--seconds','1'],env=env,capture_output=True,text=True,timeout=10)
        self.assertEqual(reopened.returncode,0,reopened.stderr)
        self.assertEqual((self.root/'parameters.v18').read_bytes(),saved)
        self.assertEqual(sorted(p.name for p in self.root.iterdir()),['parameters.v18'])

    def test_evolve_workspace_and_saved_base_parameters(self):
        if not args.player:
            self.skipTest('player required')
        samples = struct.pack('<8000h', *[int(12000 * math.sin(i * .2)) for i in range(8000)])
        (self.root / 'input.wav').write_bytes(riff([fmt(rate=8000), chunk(b'data', samples)]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        base = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                '--state-dir', str(self.root)]
        result = subprocess.run(base+['--self-test-evolve','--seconds','6'], env=env,
                                capture_output=True,text=True,timeout=12)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertIn('toggles=2',result.stdout)
        self.assertIn('help_views=1',result.stdout)
        values = (self.root/'parameters.v18').read_text().splitlines()[1].split()
        self.assertEqual(len(values),150)
        self.assertEqual(float(values[3]),.5)  # Base position is never overwritten by modulation.
        self.assertEqual(float(values[108]),3)
        self.assertAlmostEqual(float(values[109]),.06,places=5)
        self.assertAlmostEqual(float(values[110]),.25,places=5)
        self.assertEqual(float(values[111]),5)
        self.assertAlmostEqual(float(values[112]),.05,places=5)
        self.assertEqual(float(values[113]),0)
        saved=(self.root/'parameters.v18').read_bytes()
        reopened=subprocess.run(base+['--seconds','1'],env=env,capture_output=True,text=True,timeout=10)
        self.assertEqual(reopened.returncode,0,reopened.stderr)
        self.assertEqual((self.root/'parameters.v18').read_bytes(),saved)

    def test_grouped_navigation_collision_scene_and_save(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(),chunk(b'data',struct.pack('<h',10000))]))
        env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
        result=subprocess.run([str(args.player),'--sample-dir',str(self.root),'--sample','input.wav',
            '--state-dir',str(self.root),'--self-test-scene','--seconds','8'],env=env,capture_output=True,text=True,timeout=18)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        self.assertRegex(result.stdout,r'collision_hits=[1-9][0-9]*')
        self.assertIn('toggles=2',result.stdout)
        fields=(self.root/'parameters.v18').read_text().split()
        self.assertEqual(float(fields[98]),1)
        self.assertAlmostEqual(float(fields[103]),.07,places=5)
        self.assertEqual(float(fields[104]),61)
        self.assertEqual((self.root/'theme.v1').read_text(),'GrainMM UI1\n1\n')

    def test_held_dpad_repeats_and_accelerates(self):
        if not args.player:
            self.skipTest('player required')
        (self.root/'input.wav').write_bytes(riff([fmt(),chunk(b'data',struct.pack('<h',12000))]))
        env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
        result=subprocess.run([str(args.player),'--sample-dir',str(self.root),'--sample','input.wav',
            '--state-dir',str(self.root),'--self-test-repeat','--seconds','4'],env=env,capture_output=True,text=True,timeout=12)
        self.assertEqual(result.returncode,0,result.stderr+result.stdout)
        fields=(self.root/'parameters.v18').read_text().split()
        self.assertEqual(float(fields[2]),250)
        updates=int(re.search(r'updates=(\d+)',result.stdout).group(1))
        self.assertGreater(updates,8)

    def test_harmony_help_edit_reset_and_save(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        result = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                                 '--state-dir', str(self.root), '--self-test-harmony', '--seconds', '4'],
                                env=env, capture_output=True, text=True, timeout=12)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        fields = (self.root / 'parameters.v18').read_text().split()
        self.assertEqual(fields[:2], ['GrainMM', '18'])
        self.assertEqual([float(x) for x in fields[68:74]], [0, 3, 2, 2, 1, 5])
        self.assertIn('help_views=1', result.stdout)
        self.assertIn('toggles=2', result.stdout)

    def test_named_patch_and_visual_trim_workflow(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        command = [str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                   '--state-dir', str(self.root), '--self-test-library', '--seconds', '4']
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=12)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertIn('patch_saves=1 patch_loads=1 waveform_views=2', result.stdout)
        patch = (self.root / 'patches/patch-01.gmm').read_text()
        self.assertTrue(patch.startswith('GrainMM Patch 1\n"OBTCH 01"\nGrainMM 18\n'))
        fields = (self.root / 'parameters.v18').read_text().split()
        self.assertAlmostEqual(float(fields[62]), 0.01, places=5)
        self.assertFalse((self.root / 'patches/patch-01.gmm.pending').exists())

    def test_missing_patch_wav_keeps_instrument(self):
        if not args.player:
            self.skipTest('player required')
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 12000))]))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        seed = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                               '--state-dir', str(self.root), '--self-test', '--seconds', '1'],
                              env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(seed.returncode, 0, seed.stderr)
        saved = (self.root / 'parameters.v18').read_bytes()
        patches = self.root / 'patches'
        patches.mkdir()
        data = saved.decode().replace('"input.wav"', '"missing.wav"')
        (patches / 'patch-01.gmm').write_text('GrainMM Patch 1\n"MISSING FILE"\n' + data)
        result = subprocess.run([str(args.player), '--sample-dir', str(self.root), '--sample', 'input.wav',
                                 '--state-dir', str(self.root), '--self-test-patch-failure', '--seconds', '2'],
                                env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertIn('Patch operation failed:', result.stderr)
        self.assertIn('patch_saves=0 patch_loads=0', result.stdout)
        self.assertRegex(result.stdout, r'grains=[1-9][0-9]*')
        self.assertEqual((self.root / 'parameters.v18').read_bytes(), saved)
        self.assertEqual((patches / 'patch-01.gmm').read_text(), 'GrainMM Patch 1\n"MISSING FILE"\n' + data)

    def test_unknown_odd_chunks_and_data_before_format(self):
        payload = riff([chunk(b'JUNK', b'abc'), chunk(b'data', b'\0\0'), fmt(), chunk(b'LIST', b'ab')])
        (self.root / 'input.wav').write_bytes(payload)
        self.assertEqual(self.render().returncode, 0)

    def test_working_directory_cannot_substitute_sample(self):
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', struct.pack('<h', 20000))]))
        decoy = self.root / 'decoy'
        decoy.mkdir()
        (decoy / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', b'\0\0')]))
        result = subprocess.run([str(args.renderer), '--sample-dir', str(self.root),
                                 '--sample', 'input.wav', '--output', str(self.root / 'output.wav'),
                                 '--seconds', '1'], cwd=decoy, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn('peak=0 ', result.stdout)

    def test_malformed_containers(self):
        valid = riff([fmt(), chunk(b'data', b'\0\0')])
        for payload in (b'', b'RIFF', b'RIFX' + valid[4:], valid[:-1], valid + b'x',
                        riff([fmt()]), riff([chunk(b'data', b'\0\0')]),
                        riff([fmt(), fmt(), chunk(b'data', b'\0\0')]),
                        riff([fmt(), chunk(b'data', b''), chunk(b'data', b'\0\0')]),
                        riff([chunk(b'fmt ', b'a'), chunk(b'data', b'\0\0')]),
                        riff([fmt(), chunk(b'data', b'')]),
                        riff([fmt(), b'data' + struct.pack('<I', 0xffffffff)]),
                        riff([fmt(), b'data' + struct.pack('<I', 1) + b'x']),
                        riff([fmt(), chunk(b'data', b'\0\0')] + [chunk(b'JUNK', b'')] * 4096)):
            with self.subTest(length=len(payload)):
                self.reject(payload)

    def test_unsupported_formats(self):
        for format_chunk in (fmt(0), fmt(3), fmt(bits=8), fmt(code=3), fmt(rate=0),
                             fmt(rate=192000), fmt(alignment=7), fmt(byte_rate=1), fmt(2)):
            with self.subTest(header=format_chunk):
                self.reject(riff([format_chunk, chunk(b'data', b'\0\0')]))

    def test_memory_caps_before_decode(self):
        for size in (8 * 1024 * 1024 + 2, 33 * 1024 * 1024):
            with self.subTest(size=size):
                with (self.root / 'input.wav').open('wb') as stream:
                    stream.write(b'RIFF' + struct.pack('<I', size + 36) + b'WAVE' + fmt() +
                                 b'data' + struct.pack('<I', size))
                    stream.truncate(size + 44)
                self.assertNotEqual(self.render().returncode, 0)
                self.assertFalse((self.root / 'output.wav').exists())

    def test_exact_decoded_memory_cap(self):
        size = 8 * 1024 * 1024  # PCM16 -> exactly 16 MiB float32; sparse silence fixture.
        with (self.root / 'input.wav').open('wb') as stream:
            stream.write(b'RIFF' + struct.pack('<I', size + 36) + b'WAVE' + fmt() +
                         b'data' + struct.pack('<I', size))
            stream.truncate(size + 44)
        result = self.render()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('peak=0', result.stdout)

    def test_path_traversal_symlinks_and_fifo(self):
        (self.root / 'valid.wav').write_bytes(riff([fmt(), chunk(b'data', b'\0\0')]))
        for name in ('../valid.wav', '/absolute.wav', '.', '..', 'a\\b.wav', 'x' * 129):
            with self.subTest(name=name):
                self.assertNotEqual(self.render(name=name).returncode, 0)
        (self.root / 'link.wav').symlink_to(self.root / 'valid.wav')
        self.assertNotEqual(self.render(name='link.wav').returncode, 0)
        os.mkfifo(self.root / 'fifo.wav')
        self.assertNotEqual(self.render(name='fifo.wav').returncode, 0)
        (self.root / 'dirlink').symlink_to(self.root, target_is_directory=True)
        self.assertNotEqual(self.render(name='valid.wav', directory=self.root / 'dirlink').returncode, 0)

    def test_writer_preserves_existing_and_symlink_outputs(self):
        (self.root / 'input.wav').write_bytes(riff([fmt(), chunk(b'data', b'\0\0')]))
        protected = self.root / 'protected'
        protected.write_bytes(b'preserve')
        self.assertNotEqual(self.render(output='protected').returncode, 0)
        self.assertEqual(protected.read_bytes(), b'preserve')
        (self.root / 'link.wav').symlink_to(protected)
        self.assertNotEqual(self.render(output='link.wav').returncode, 0)
        self.assertEqual(protected.read_bytes(), b'preserve')


if __name__ == '__main__':
    unittest.main()
