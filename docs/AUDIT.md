# Initial release audit (historical)

> This is the 2026-11-17 baseline audit. Device tests, permission and storage
> layout have since changed. Use [current status](PORTING_PROGRESS.md),
> [legal/provenance](LEGAL_AND_PROVENANCE.md) and [layout](development/REPOSITORY_LAYOUT.md).

The original payload has since been relocated at the user's request into
ignored reference storage; all 440 baseline inputs remain verified. The original
README is preserved there and in Git history. See development/REPOSITORY_LAYOUT.md.

# Repository audit — 2026-11-17

Static review of local release payload, with independent host/device-proof work added afterward.
No original GrainBrick executable was executed. No physical MM+ test occurred.

## Baseline and preservation

Starting commit `5d4aba8a08e5d18a10ed9608b6d6ad2030f18f8a`, branch main, remote
https://github.com/lkotlarenko/GrainMM.git. Four README-only commits; only `README.md`
tracked. GitHub repository API reported admin/push permissions available; these
were not used to publish. 440 non-Git input files, 72,670,780 bytes including the private guiding
files. All other original payload files were untracked. No cleanup or history
rewrite is safe. Private `.local/baseline/` holds full initial Git status, log,
tracked paths, SHA-256 manifest, imports, ELF metadata and a 36,176,981-byte
compressed snapshot including Git history. It is a local recovery copy, not an
off-machine backup. The payload remains in place; `.gitignore` isolates it
without moving or deleting any input. Guiding files remain private and ignored.

`REFERENCE_INVENTORY.json` inventories every non-private input path/checksum,
all ELF files and each application's top-level imports. Archives were listed,
not executed; the SoundTouch archive contains genuine bridge source plus
SoundTouch upstream and Debian source archives. No standalone C/C++ implementation
source is present outside vendor NumPy headers. No general native build system
or tests existed before this work.

## Verified counts and corrections

118 non-vendor Python files; **59,057 splitlines**, differing from the research's
59,175 (the research appears to count one extra line per file). Hotspots are one line shorter than the research: `main.py` 4,645,
`ui/renderer.py` 10,323, `engine/control.py` 3,232, `engine/audio_worker.py` 1,864.
47 runtime ELF objects: **46 AArch64 and one x86-64**. The research's blanket
AArch64 observation misses `visualizers/native/gb_mandala_x86_64.so`. Root has
22 shared libraries, including SoundTouch. Five WAVs total 20,756,024 bytes
(19.79 MiB), stored 48 kHz stereo PCM16. Assets' authorship/redistribution
terms remain unverified.

## Module map and dependency flow

`main.py` -> UI renderer/state/input -> control/parameter bus ->
`AudioWorker` process -> DSP chain/granular/oscillator/ORBIX/effects ->
ctypes GrainBrick shared libraries -> PCM conversion -> aplay/ALSA.
Samples/project JSON/settings feed control and DSP; shared arrays carry
parameters/meters across processes; visualizers consume audio statistics.

| Area | Local evidence and impact |
| --- | --- |
| Entry/UI | main.py initializes Pygame; renderer.py uses 1024x768 logical geometry; display_adapter.py:109 smoothscales on presentation. |
| Audio | main.py:53-54 sets 48000/512; audio.py sets 7168-frame ALSA buffer and 512 period (149.3 ms buffer, 10.67 ms block; not an end-to-end latency measurement). aplay runs through subprocess pipes. |
| Worker/control | audio_worker.py uses multiprocessing RawArray/RawValue and NumPy views; per-refresh lists/byte copies. Memory duplication and scheduling require measurements. |
| Native boundary | granular_c.py:19 CDLL import and other wrappers require incompatible binaries at startup; no build sources for core DSP. |
| Input/system | input.py uses device-specific numeric joystick buttons; button_mapping.py:30-32 repeats ACTIONS assignment; cpu_performance.py writes cpufreq minimum in STABLE mode (AUTO leaves it unchanged). |
| Sample/runtime | sample.py uses NumPy and ffmpeg for MP3; ALSA live paths use ctypes; BLE integration imports dbus/gi and depends on BlueZ. |
| Storage/assets | projects, settings, recordings and samples intermix release data and mutable state. Keep entirely out of new packages. |

Direct non-standard Python imports include pygame, numpy, dbus and gi; internal
packages engine/ui/visualizers/claude_gfx use ctypes, subprocess, multiprocessing,
shared memory and threads. External programs include aplay, arecord, ffmpeg,
fbset and platform-specific helpers (inspect per-file import inventory and
subprocess call sites before pursuing Python). Bundled NumPy 1.26.4 CPython3.12
AArch64/OpenBLAS cannot be used by ARMv7 Python. Python/Pygame runtime availability
on this user's Onion installation has not been demonstrated.

## Ranked risks / suspected defects

1. Critical: accidental deletion or staging of untracked original payload and
   private guides. Fixed for new work with snapshot, ignores and exact source gate.
2. Critical: AArch64 DSP/NumPy ABI prevents supplied app startup on ARMv7.
   Confirmed ELF headers; no simple rebuild without source. New native proof is separate.
3. High: no public blanket license for original app; component notices are not
   an application license. Public port encouragement is recorded separately.
4. High: CPU policy writes/watchdog and KNULLI launcher mismatch must not carry over.
5. High: full Python/NumPy/multiprocessing memory and block-copy costs threaten
   128 MiB budget. Risk only; no MM+ profiling supports numeric CPU/RSS claims yet.
6. Medium: 1024x768 full-canvas scaling wastes work at 640x480. Native proof
   renders directly at 640x480, 25 fps software surface.
7. Medium: hard-coded joystick codes conflict with Onion button contract.
   Proof uses Onion keymap A=SPACE, MENU=ESCAPE; real key behavior still unverified.
8. Low: repeated ACTIONS assignments are redundant. Original code left unchanged.

## Complete runtime ELF inventory

All non-vendor GrainBrick objects lack corresponding standalone source in this
payload, except the MIT SoundTouch bridge source inside its compliance ZIP.
The x86 mandala, smoothscale and live binaries also lack implementation source.
NumPy source is not bundled; SoundTouch source archive is present. Licenses are
in LEGAL_AND_PROVENANCE.md; ELF path hashes are in REFERENCE_INVENTORY.json.

| Path | Class | Machine |
| --- | --- | --- |
| `libgrainbrick_comp.so` | ELF64 | AArch64 |
| `libgrainbrick_eq.so` | ELF64 | AArch64 |
| `libgrainbrick_fullopt.so` | ELF64 | AArch64 |
| `libgrainbrick_fullopt_s2.so` | ELF64 | AArch64 |
| `libgrainbrick_fx.so` | ELF64 | AArch64 |
| `libgrainbrick_fx_fo2.so` | ELF64 | AArch64 |
| `libgrainbrick_granular.so` | ELF64 | AArch64 |
| `libgrainbrick_granular_live.so` | ELF64 | AArch64 |
| `libgrainbrick_granular_live_fo1.so` | ELF64 | AArch64 |
| `libgrainbrick_live.so` | ELF64 | AArch64 |
| `libgrainbrick_orbix.so` | ELF64 | AArch64 |
| `libgrainbrick_orbix_fo2.so` | ELF64 | AArch64 |
| `libgrainbrick_orbix_rev23.so` | ELF64 | AArch64 |
| `libgrainbrick_oscillator.so` | ELF64 | AArch64 |
| `libgrainbrick_resonator.so` | ELF64 | AArch64 |
| `libgrainbrick_shimmer_native.so` | ELF64 | AArch64 |
| `libgrainbrick_shimmer_voice2.so` | ELF64 | AArch64 |
| `libgrainbrick_shimmer_voice2_fo1.so` | ELF64 | AArch64 |
| `libgrainbrick_soundtouch.so` | ELF64 | AArch64 |
| `libgrainbrick_stretch.so` | ELF64 | AArch64 |
| `libgrainbrick_tape.so` | ELF64 | AArch64 |
| `libSoundTouch.so.1` | ELF64 | AArch64 |
| `claude_gfx/libgb_claude_smoothscale.so` | ELF64 | AArch64 |
| `visualizers/native/gb_mandala_aarch64.so` | ELF64 | AArch64 |
| `visualizers/native/gb_mandala_x86_64.so` | ELF64 | x86-64 |
| `native/live/libgrainbrick_live.so` | ELF64 | AArch64 |
| `_vendor/numpy.libs/libgfortran-daac5196.so.5.0.0` | ELF64 | AArch64 |
| `_vendor/numpy.libs/libopenblas64_p-r0-17488984.3.23.dev.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_multiarray_tests.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_multiarray_umath.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_operand_flag_tests.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_rational_tests.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_simd.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_struct_ufunc_tests.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/core/_umath_tests.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/fft/_pocketfft_internal.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/linalg/_umath_linalg.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/linalg/lapack_lite.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_bounded_integers.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_common.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_generator.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_mt19937.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_pcg64.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_philox.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/_sfc64.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/bit_generator.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
| `_vendor/numpy/random/mtrand.cpython-312-aarch64-linux-gnu.so` | ELF64 | AArch64 |
