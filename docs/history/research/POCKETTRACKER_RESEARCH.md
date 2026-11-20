# PocketTracker research

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Reviewed 2026-11-18 at commit d0c2ab569430c664a2106de577a9b8881df77f71.
Primary source: [conanizer/pockettracker](https://github.com/conanizer/pockettracker/tree/d0c2ab569430c664a2106de577a9b8881df77f71).
This is source inspection, not a claim that we tested that app on our Miyoo.

## Findings and decisions

- The README explicitly lists an OnionOS ARM hard-float Mini/Mini+ package and
  a 640×480 design. It bundles SDL2; GrainMM keeps its physically verified SDL1.2
  adapter. Another project's backend does not establish compatibility for our app.
- `shell/toolchain-miyoo.cmake` uses the same GCC8.3/glibc2.28 SDK family, naming
  Cortex-A7 and NEON/VFPv4. Our conservative flags remain unchanged; no unmeasured
  optimization or system-level change follows from this comparison.
- `native/effects/modules/drive-module.h` separates parameter updates from sample
  processing and crossfades changed settings. It bypasses settled inactive drive.
  GrainMM already prepares outside the callback and smooths parameters; retain
  exact bypass in the new tape effect and verify transitions numerically.
- `native/effects/primitives/bus-sleep.h` waits for an input/output quiet span
  covering the delay before sleeping. Do not gate reverb/delay on input silence
  alone: that would truncate or strand tails. No new sleep gate is introduced
  without timing and tail evidence. Tape has no feedback and clears after bypass.
- `CREDITS.md` separates libraries, algorithm references, contributors and
  inspiration. Adopt that clarity while putting elly's original native work first.
- License is GPL-3.0; the credits distinguish mixed-license DSP dependencies,
  including MIT/LGPL DaisySP files and GPL reverbs. No code, assets or binaries
  were imported. The tape module is independently authored using elementary
  soft clipping, a one-pole filter and modulated fractional delay. This research
  does not change GrainMM's source license or third-party inventory.

## Initial alpha8 follow-up (historical)

Compare low-cost parameter smoothing, tail-aware idle work, contextual effect
help and offline rendering workflows when they resolve a measured GrainMM need.
Do not import a large dependency tree merely to gain an effect label. For the
current iteration, deliver one working tape page, state migration and bounded
physical validation before starting another effect batch.

## Alpha12 effects audit and selection

The pinned tree contains crush, filter, drive, limiter, ott, eq, delay, reverb and
Dust chain modules. The exact [crush interface](https://github.com/conanizer/pockettracker/blob/d0c2ab569430c664a2106de577a9b8881df77f71/native/effects/modules/crush-module.h)
was retrieved and read:2811bytes, bit reduction and sample-rate reduction with
state transitions; it uses DaisySP decimators. No implementation was imported.

| Candidate | GrainMM decision |
| --- | --- |
| Bitcrusher | Implement now: original signed quantizer/sample hold, tone, drive, mix/output; exact dry bypass |
| Filter | Implement now: original smooth low/high/bandpass SVF; useful tonal control beyond static EQ |
| Drive | Existing tape saturation already covers a useful subset; do not add a duplicate label |
| Limiter | High-value next safety feature; distinguish it from current compressor/hard PCM clamp and verify transients/latency |
| OTT | Interesting multiband upward/downward compression; later, after crossover/gain/headroom and CPU tests |
| Dust chain | Creative texture inspiration; later per-component review and budget, not wholesale dependency import |
| Chorus | Additional standard effect authored for GrainMM in alpha12; not attributed as a PocketTracker module |

Pinned tree metadata remains local. Source is inspiration and comparison, not
proof our independently authored effects sound identical or use those components.
Keep component notices if future code reuse occurs. See FEATURE_COMPARISON.md for
sequencer/oscillator/engine gaps that effects alone do not resolve.
