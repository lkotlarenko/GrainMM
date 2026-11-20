# Historical candidate — superseded by alpha13

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


Physical feedback found the simultaneous reset chord unreliable; it is replaced
by SELECT → START → X confirmation. Measurements below describe alpha12 only.

# Alpha12 development baseline

2026-11-18. Version 0.1.0-alpha.12; release 0239b0f887ee.
App SHA256:00d3426493984a902d03fd45d54918bd3e9b4681846ff388c702ca86eeff161a.
Installation ZIP145929bytes, independently repeated byte-for-byte. Source remains private.

## Scope

Three original effects: signed quantizer/sample hold crusher, smooth multimode
state-variable filter and fixed32KiB stereo chorus. Fifteen pages,90controls,
settings10, preserved older settings and patch wrapper compatibility. Hold SELECT+X
for1.5seconds to reset the processing rack/tails while preserving sources and
prior play/pause. Browser/patch dialogs and focus loss cancel the gesture.

Typed shared parameter metadata removes duplicated editor/codec switches/lists.
Editor373→72lines and codec179→74lines, plus225line shared metadata. Static page/
schema counts and all-page help tests prevent silent missing-control metadata.
Full source/inspiration comparison and delivery sequence: FEATURE_COMPARISON.md.

## Host, static and build evidence

- Final host21 and ASan/UBSan/LSan20 CTest cases passed; leak checks outside ptrace.
- Runtime5/tooling8/hardware-tooling8/source-archive2 passed.
- Clang analyzer passed crusher/filter/chorus/schema/codec. Warnings-as-errors
  host/SDK ARM builds pass; no new runtime shared-library dependency.
- Numeric tests: independent4-bit quantization, filter rejection, chorus delay/
  feedback impulse positions, three rates, block equivalence, exact settled dry
  bypass, finite bounds and no allocations in active process paths.
- Gesture tests cover short holds, duplicate events, one-shot/rearm and tick wrap.
  Full dummy event probes exercise actual reset/source-preservation and persistence.
- Old states1–9 and embedded old patches are tested through the shared strict codec.
  New schema10 patches need a compatible app; old app rollback uses preserved v9.
- Development caught missing help tips after adding pages (null string), now guarded
  by deduced array size/static assertion and all-page traversal. Early failed builds/
  stale overlapping logs are not acceptance evidence; final acceptance logs are
  distinct. Future test runs must follow a successful build and use separate logs.

## Physical ARM numerical suite

texture_test compiled to SHA256
f50f5b82f4ee12bed26d13e1f8c9fe19261e533312ac9e38306e0dcf1c1a8901
passed within the approved app state directory. Exact file hash/type/node count
checked and temporary file/directory removed individually. It performs no file I/O.
No host-style library/preset filesystem tests were run outside the device boundary.

## Native dummy-driver application checks

Stock Onion preload retained. SDL dummy video/audio,48kHz/stereo/1024frames.

| Test | Result | Callback maximum | Late hints |
| --- | --- | --- | --- |
| New effect pages/save/migration | Expected18new fields, v9 preserved |31112us |1 |
| Reopen settings10 | Saved bytes unchanged |966us |0 |
| Guarded dry reset | Exactly1reset, all rack defaults, source/grain/OSC/harmony unchanged |42530us |3 |
| Patch/waveform regression |1save/1load/2waveform views, preparation11593us |16758us |1 |
| Alpha11 rollback | Preserved v9 loads, v10 unchanged |429us |0 |

Owner WAV/v9 settings hashes matched; eight fixture files/six directories checked
and removed. Saved owner patches were never modified by these tests. No screenshots.
Callback late hints are not XRUN counts. Large maxima remain an investigation item;
functional completion is not physical sound-quality proof.

## Offline native performance

One-second workloads,93blocks of512frames each, stock background services, SSH and
owner-set VNC2FPS. CPU clock was read, never changed. All11default graph checksums
match alpha11 exactly. New effects are exact dry bypass at default MIX0.

| Dense full rack | p95 | Maximum | Checksum |
| --- | --- | --- | --- |
| Original rack, new effects off |5987.667us |23859.666us |869 |
| New effects+harmony on |6268.167us |15415us |905 |
| Same enabled graph, repeat |6153.5us |10608.833us |905 |

The512frame nominal interval is10666.7us. Both dense graphs spawn70grains/drop9
from fixed-pool saturation;0software clipping. One non-dense subset produced
p9513358.833us/max35411.167us; the repeat dense subset without resonator/rhythm
also showed p958638.167us/max22794.334us. Workload subsets have inconsistent timing
outliers, so no isolated overhead/speedup or worst-case deadline guarantee is made.
VNC-off/profile investigation and actual listening remain required. Do not hide
those outliers by reporting only the best all-on p95.

## Device storage and owner acceptance

Only alpha12 and verified alpha11 rollback remain, whole app3264KiB including
owner data before listening. Alpha10's exact app-owned files were hash-checked
against the preserved host archive before individual removal. No broad deletion,
service, mixer, CPU, firmware or other-device-file changes occurred.

Owner confirmed the effects work, but reported SELECT+X did not behave as expected.
The captured foreground session recorded11743callbacks/1024,worst13030us,late0,
software clips0,3728grains/no drops,354updates,13help views and14reset events.
Those counters do not override the owner's usability report. Keep alpha11
as LAST_GOOD until owner confirmation and final logs/state checks. Long stress,
headphones/power/sleep/VNC-off checks and public native publication remain gated.
