> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Portable EQ validation — 2026-11-18

This module is implemented and tested but **not connected to the current app's
controls/audio graph yet**. The verified alpha3 build is unchanged. Architecture:
[ADR0006](../../adr/0006-portable-equalizer.md). No original DSP code was copied.

## Reproduce

```sh
sh scripts/build.sh host-debug
sh scripts/build.sh host-asan-ubsan
GRAINMM_SDK="$PWD/.local/sdk/miyoomini-toolchain" sh scripts/build.sh miyoo-armv7-release
build/host-debug/equalizer_benchmark --seconds 3
```

The benchmark is built for host/ARM and excluded from installation ZIPs. Execute
the ARM build on hardware only through the approved isolated workflow. Each case
renders281blocks of512frames at48kHz for --seconds3 (integer-rounded simulated
duration), without SDL, audible output, files or live input. Coefficients are
prepared before timing; changing case includes set_coefficients every8blocks.
Default3, accepted1–5seconds per case. It is not an audio soak or CPU% sample.

For the numerical suite on ARM, using the pinned SDK:

```sh
.local/sdk/miyoomini-toolchain/bin/arm-linux-gnueabihf-g++ \
  --sysroot=.local/sdk/miyoomini-toolchain/arm-linux-gnueabihf/libc \
  -std=c++17 -O2 -mcpu=cortex-a7 -mfpu=vfpv3-d16 -mfloat-abi=hard \
  -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -Iinclude \
  src/dsp/equalizer.cpp tests/equalizer_test.cpp \
  -o build/miyoo-armv7-release/equalizer_test
python3 scripts/toolchain/validate_elf.py build/miyoo-armv7-release/equalizer_test
```

The suite exited0 on the physical Miyoo. SHA256:
`5150551a1b9c198bd36417e3400d62aa15bd580947c3c6fe192c36e5912bfe3e`.
It covers8/44.1/48/96kHz, analytic and processed responses, block invariance,
channel isolation, reset, invalid coefficients/PCM, allocation-free methods and
latest-target transitions. Native unit/benchmark files were hash-checked, run
with a15second device timeout, then removed by exact name. No SD residue remains.

## Measured native cost

Miyoo Mini Plus/Onion4.3.1-1, SDKv0.0.3 GCC8.3 -O2, Cortex-A7/VFPv3-D16/hard.
VNC2FPS and SSH enabled; foreground GrainMM closed. No governor, clock, audio or
system changes. CPU frequency is dynamic: before800000kHz, after1200000kHz for
this run. Do not use these short measurements as WCET or a fixed-clock comparison.

Final benchmarkSHA256:
`0e87dd75191b11598ac798e830cde0a8c68ce3179a4fc64a5ecd23d4c29de022`.
Equalizer object1760bytes on ARM; no heap storage in the processor.

| Case | p95 us/512 | p99 us/512 | Maximum us/512 |
| --- | ---: | ---: | ---: |
| Flat, cached bypass | 65.167 | 236.834 | 1166.000 |
| Six bells, alternating ±12dB | 657.500 | 1037.333 | 1347.167 |
| Switching all bands/Q | 837.000 | 1089.500 | 1120.500 |

Before cached bypass, double processing measured flat p95728us, six bells454.167us,
changing821.333us with1200000kHz before/after. Runtime/background variation prevents
a precise percentage speedup claim. Flat bypass removes the recursive work and
preserves output checksums; six-bell/changing checksums also remain unchanged.
The earlier float version measured different clocks and is not a controlled
speed comparison. It was rejected for a measured0.590600dB low-frequency/high-Q
coefficient error; double fixes it without needing a dependency or SIMD path.

No combined granular/delay/EQ graph, audible EQ, XRUN, end-to-end latency or
VNC-disabled baseline is claimed. Current appSHA256 stays
`f6cdc3f767cd15a0f3051fbf8097849ba154cbe5f32080379e423d86b62b8a29`.

## Reference investigation

A private bounded ARM64 emulator can now invoke the original delay/filter exports
for behavioral experiments. A left0.5 impulse through a10ms fully-wet delay
appeared at frame480 at48kHz. Legacy filter mode/float argument interpretation and
its relevance to the current reference chain remain uncertain; do not infer
full matching from export names. The current EQ contract is the six-band model,
not that legacy export. Guest code receives no host syscall passthrough; memory,
instructions and time are bounded. Host libm hooks mean this is not bit-exact
original-hardware evidence. Dependency pins/results and unchanged binaries remain
in private reference storage; no emulator or foreign library ships with GrainMM.
