# Building

Host prerequisites: C++17 compiler, CMake >=3.21 (presets), Ninja, pkg-config,
SDL2 development files, Python >=3.12 for tooling. This host: GCC16.2.1,
CMake4.5.0, pkg-config SDL2.32.74, x86_64 Linux. No dependencies are installed
globally by project scripts.

```sh
sh scripts/build.sh host-debug
sh scripts/build.sh host-asan-ubsan
python3 tests/tooling_test.py
python3 scripts/maintenance/check_source.py
```

LeakSanitizer cannot operate under ptrace. If a sandbox reports that limitation,
rerun tests with approved execution outside it; never count a failed test as passed.

```sh
python3 scripts/toolchain/fetch_sdk.py
GRAINMM_SDK="$PWD/.local/sdk/miyoomini-toolchain" sh scripts/build.sh miyoo-armv7-release
python3 scripts/packaging/package.py --local-experiment
python3 scripts/packaging/package.py --validate dist/GrainMM-local-proof.zip
```

The maintenance cleanup retains the compressed SDK and removes its reproducible
extraction. Run `fetch_sdk.py` before the next ARM build: with the cached archive,
restoration is offline. Budget about 1.5 GiB for extraction. See
[workspace maintenance](development/WORKSPACE_MAINTENANCE.md).

SDK fetch uses gh only when the archive is missing, verifies the pinned archive SHA256 before extraction, writes
only .local/sdk, and never overwrites an extracted SDK. Review TARGET_PLATFORM.md
and development/MIYOO_HARDWARE.md for provenance, verified CPU features and sysroot limitations.
CMake cross profile confines pkg-config, headers and libraries to sysroot;
no host SDL is accidentally linked. No march=native, AArch64 or GL flags.
Use a fresh build directory after SDK changes. Record SDK hash, source commit,
compiler flags and artifact checksum for every device test.

Package creation refuses to overwrite an existing ZIP. Choose a new --output
path for a subsequent experiment. Fixed ZIP member timestamps make identical
inputs deterministic; full compiler/environment bit reproducibility has not
been claimed. The SDK path can appear in build metadata. SDK/host tool versions
are pinned or recorded, but no hermetic container image has been built yet.
Prepared CI checks host/sanitizers/cross/package allowlist and uploads installation ZIPs
and matching GPL source/checksums. Authorized version-tag pushes trigger the prepared release workflow; see
[RELEASING.md](development/RELEASING.md).

Current granular/offline usage: GRANULAR_PROTOTYPE.md. CTest now runs WAV,
exact PCM, granular, parameter-editor and queue checks as well as the tone/lifecycle
proof. -Werror is enabled by default. granular_benchmark is built for both host
and ARM; it opens no display/audio device. Use `--seconds 5`
for a short offline CPU sweep. A default host-debug benchmark is unoptimized;
use an explicitly recorded -O2 build for performance comparisons.

Instrument graph benchmark: `build/miyoo-armv7-release/instrument_benchmark --seconds 3`
uses synthetic stereo data and opens no hardware audio/display. It is a bounded
offline workload, not an audible soak. Actual device method/results are in
development/RESUME.md and versioned baseline reports. Additional expf requires GLIBC2.27; the verified
Onion target has glibc2.28. Do not substitute SDK libraries on other devices.

UI-only dummy benchmark (no real audio/display):

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy build/host-debug/ui_benchmark --requests 2000
```

Use identical compiler flags/workloads for comparisons. This does not measure audio
callbacks or physical display transfer costs. The benchmark caps requests at 2,000.


Full-instrument offline export from an existing settings directory:

```sh
build/host-debug/grainmm-export --state-dir /path/to/state --seconds 15
```

The tool resolves the saved WAV beneath `--sample-dir` (defaults to the state
directory), or uses saved built-in sources. It writes an exclusive numbered file
in the state's `exports` folder and opens no SDL/audio device. This is a fresh
settings render; `grainmm-render` remains the older granular diagnostic.
