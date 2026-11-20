# Feature roadmap

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Current state and exact acceptance are in [PORTING_PROGRESS.md](../../PORTING_PROGRESS.md)
and [RESUME.md](../../development/RESUME.md). Feature usefulness and sound matter more than matching
GrainBrick line-by-line or bit-for-bit. Original GrainBrick by ETRNLIAM; GrainMM
created and maintained by elly (@lkotlarenko).

| Area | Implemented | Remaining |
| --- | --- | --- |
| Grains |16voices, pitch/position/spread, reverse/random motion, scan, region/window/width, musical chord patterns | Independent layers and measured resampling improvements |
| Synthesis/FX | Oscillator, collision instrument, drone/events, EQ, delay/reverb, tape, compression, resonance, rhythm, crusher/filter/chorus; alpha23 OUTPUT candidate | Shimmer, additional synthesis/resonator types and routing |
| Media/state | Rooted WAV library, schema16/132 controls, named16slot patches, waveform trim | Named-file import/export tools and verified recording hardware |
| UI | Upright640×480, parameter/help/browser/patch/name/waveform views | More usability/power/sleep/input coverage |
| Performance | Bounded audio work, fixed/prepared controls, cached rendering, measured native workloads | Longer stress/headphones, VNC-off comparison and scheduling outliers |
| Distribution | Pinned SDK, exact source/package archives, checksums, GPL notices and publication gate | Hosted CI and explicit source/binary publication authorization |

This is not full GrainBrick project compatibility or exact ORBIX/Rings behavior.
No USB/BLE/mic capability is assumed. No fictional progress percentage is assigned.
Private reference payload is preserved; original native libraries are incompatible
ARM64 inputs, not the runtime. Future reuse needs per-component license review.

Next iterations should validate patch/media usability and reliability,
then manual slice editing, offline export and richer synthesis within measured CPU/memory limits. Do not expand scope
by changing Onion services, mixer, governor, firmware or unrelated SD files.

Detailed reference comparison, concrete next batches and faster delivery method:
[FEATURE_COMPARISON.md](FEATURE_COMPARISON.md).
