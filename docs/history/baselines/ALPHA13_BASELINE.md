# Alpha13 development record

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-18. Version0.1.0-alpha.13; release0e2cd2a2f2fe.
AppSHA256:38bcc926f4d6f35a6b26f39ea8030df9b61ba9ca21f07fec6469e4091c3f1876.
ZIP146293bytes; repeated build is byte-identical. Native source/binaries remain private.

## Larger batch and physical correction

This batch adds the original bitcrusher, multimode resonant filter and stereo
chorus,15pages/90controls/settings10, typed shared parameter metadata and a safe
all-effects reset. Feature comparison/next delivery batches: FEATURE_COMPARISON.md.

Alpha12's simultaneous SELECT+X hold passed synthetic tests but the owner reported
it unreliable. Alpha13 removes that hold/timer implementation. SELECT opens help;
START shows a dedicated all-FX confirmation; X applies; B/SELECT cancels. Normal
help X still resets one control. No simultaneous button timing is required.
The actual event test cancels, reopens, confirms and injects repeated X-down:
exactly one reset; source/grain/OSC/harmony and prior play/pause remain intact.

DSP/audio reset/schema source is byte-identical to the already tested alpha12
source archive; only input/display/diagnostics/version changed. Alpha12's detailed
numerical/performance evidence remains in ALPHA12_BASELINE.md. No favorable new
performance claim is inferred from the UI-only correction.

## Verification

- Final21host and20ASan/UBSan/LSan CTest cases pass, leak checks outside ptrace.
- Alpha12 batch runtime5/tooling8/hardware8/source2; same tooling retested for final
  review. Analyzer/format, SDK ARM/dependencies, package integrity and repeat checks.
- Full source rebuild in alpha12 passed21tests; final alpha13 source checked separately.
- Physical ARM numeric suite verified all three unchanged DSP modules, allocation-
  free block consistency, quantization/filter/chorus references. No imported DSP.

## Native application fixtures

Stock preload retained, SDL dummy video/audio,48kHz/stereo/1024frames.

| Case | Result | Maximum | Late hints |
| --- | --- | --- | --- |
| Texture pages/save/v9 migration | Expected18new values |20801us |1 |
| Reopen | Saved bytes unchanged |947us |0 |
| Cancel/reopen/confirm/repeated X | Exactly1reset, rack dry, source fields unchanged |20503us |2 |
| Patch/waveform |1save/1load/2waveform views |27488us |2 |
| Alpha11 rollback | Reads old v9 without changing new v10 |442us |0 |

Owner WAV/v9/v10 hashes matched before/after. Eight fixture files/six directories
checked and removed individually. No screenshots. Dummy completion is not sound
quality evidence. Late hints are not XRUN counts; alpha12 offline outliers remain.

## Physical acceptance / storage

Owner confirmed alpha12's effects work; its shortcut report triggered this fix.
The foreground alpha12 session had11743callbacks/1024,worst13030us,late0,clips0,
3728grains/no drops,354updates,13help views/14reset events. Those counters do not
substitute for the owner's interaction report. Alpha13 confirmation retry pending.

Only alpha13 and verified alpha11 rollback remain: whole app3296KiB before final
owner retry. Alpha10 and superseded alpha12 files were individually hash-checked
against host archives and pruned; immutable artifacts/history remain on the host.
PREVIOUS explicitly points to verified alpha11, not the superseded alpha12.
No service/mixer/CPU/network/firmware/unrelated-file changes. Power/headphones/
long stress/VNC-off profiling and native-publication approval remain separate gates.
