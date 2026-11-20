# Alpha11 development baseline

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-18. Version0.1.0-alpha.11; installed release fd11bf924d4e.
AppSHA256:e4b75c14a574b1e69fdedb9e7dcc2e1f42a80fb2285b684fd38d81559a0f082e.
ZIP132937bytes; staged repeat is byte-identical. Source and binaries remain private.

## Features and compatibility

HARMONY adds six controls for musical per-grain pitch patterns within the same
16voice pool/sample buffer. At most nine pitch ratios are prepared outside audio;
selection/random detune is per successful spawn, not per output frame. Independent
RNG and default-unity ratios preserve old output. No borrowed DSP or new dependency.

Help previously advertised editing/page actions it ignored. Those actions now work
for parameter views; X resets just the selected control and its value is displayed.
Help over file/patch dialogs disables sound editing and explains how to return.
Cache keys include field/value/edit availability. MENU remains guaranteed exit.

Settings9 adds six fields (72total), preserves v1–v8, never falls back from corrupt
newer state. Named patch format1 reads both previous8 and new9 payloads. Old apps
cannot read newly saved9 patches; keep whole-state backups before replacing slots.

## Verification

- Full20host and19ASan/UBSan/LSan CTest cases passed. Leak checking outside ptrace.
- Runtime4/tooling8/hardware-tooling8/source-archive2 passed.
- Clang analyzer passed harmony and changed codec; warnings-as-errors host/ARM builds.
- Harmony tests compare independent interval references, chord/order sequences,
  deterministic reset/randomness, block-size invariance, unchanged disabled output
  and unchanged spawn/drop counts. Granular apply/render/reset allocation count
  remains unchanged with active harmony. Legacy patch8 and settings8 migration pass.
- Display tests cover help mode/selection/value cache invalidation. Real SDLdummy
  event sequences edit/reset help, save72fields, load patches and close normally.
- Development caught an overload ambiguity requiring explicit typed bounds fixtures,
  old page-wrap assumptions and a cache test attempting to increase END above1.
  Those fixtures were corrected; final full suites pass. No checks were disabled.

## Physical ARM, bounded silent fixtures

Stock padsp retained; SDL video/audio dummy,48000Hz/stereo/1024frames. All files
were confined to a fresh approved app state fixture. Owner WAV/v8 settings hashes
matched before/after. Seven known files/six directories were checked and individually
removed; no fixture remains. No capture or image inspection used.

| Case | Result | Callback maximum | Late hint / clips |
| --- | --- | --- | --- |
| Harmony/help/reset/v8→v9 | Six expected harmony values,2toggles,11updates |20324us |1 /0 |
| Reopen v9 | Committed bytes unchanged |429us |0 /0 |
| Alpha10 rollback reads preserved v8 | Both settings files unchanged |472us |0 /0 |
| Patch/waveform regression |1save/1load,2waveform views,source preparation9295us |20267us |1 /0 |
| Built-in rack | Save/exit normally |4504us |0 /0 |

Late hints are not XRUN counts. These dummy runs do not establish sound quality.
Two hints remain recorded; current backend exposes no hardware underrun metric.

## Offline native graph budget

One-second virtual workloads,93blocks of512frames per case. VNC owner-set2FPS,
SSH enabled. Frequency snapshots1000MHz before default and1200MHz before/after
harmony; no CPU policy changes. Short non-isolated results, not a speed comparison.

| Dense full effects/tape/resonator/rhythm | p95 | Maximum | Checksum |
| --- | --- | --- | --- |
| Harmony off |5004.666us |7826.500us |869 |
| Harmony major/bounce/two octaves/detune12ct |5024.833us |5500.166us |1618 |

Both report70spawned/9dropped from fixed-pool saturation,0software clipping.
Normal full graph p953385.166us off /3934us on; a harmony run outlier7676.666us
remains. All11default graph checksums match alpha10 exactly on physical ARM.
No assertion that adding harmony improves performance, or that p95 bounds worst case.

## Physical owner acceptance and storage

Owner confirmed alpha10 waveform trimming and named patch save/load and left the
app closed. Its late retained maximum22797us remains documented. Alpha10 promoted
as verified LAST_GOOD before alpha11. After native checks, all16alpha9 app files
were hash-matched against its preserved host archive and removed individually.
Only alpha11 and alpha10 remain; whole app3200KiB before final owner interaction.
Owner samples/settings/patches remain untouched outside explicit user interaction.

Owner confirmed harmony/help controls, clean sound and saved settings. The final
normal dsp-driver sessions recorded5669callbacks/1024,worst20178us,late0,clips0,
2153grains/no drops,81updates and1help view over127660ms; next reopen/exit recorded
450callbacks,worst14844us,late0,clips0,195grains/no drops. Final harmony fields were
transpose6,chord3,order3,octaves1,chance1,detune0. Older v8 remains intact.

One short live snapshot measured RSS6960KiB,high-water8064KiB,3threads. The process
ended before a second sample, so no CPU average is derived. This is not a stress
memory bound. LAST_GOOD promoted to fd11bf924d4e after closed-app/log/state checks;
alpha10 remains the explicit previous rollback. Final app size3232KiB with user data.
An extracted source archive independently built and passed all20host tests.

Final documentation review expanded the sound guide and corrected its version-list
punctuation. A separate reviewed host installation/source archive includes those
prose fixes; it has the same tested executable and does not replace the immutable
installed release or write another version to SD. Earlier local artifacts remain
preserved. No native source/workflow/tag/binary publication occurred.

 Longer combined stress, headphones,
power/sleep and VNC-off profiling remain. No stable/full-parity/public release claim.
