# Alpha14 development record

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-18. Version0.1.0-alpha.14; installed a76d9234be34.
AppSHA256:134757d91da6f84d17fbe1d8266ffc457980f8b9f149de3fdde22ad0febbb05a.
ZIP149893bytes, byte-identical repeat. Source/native binaries remain private.

## UI and requested shortcut

Owner confirmed alpha13 reset works but found its access sequence too cumbersome,
and requested a held START+SELECT gesture plus a better UI/Apps appearance.

Alpha14 uses six parameter cards with bounded value bars, source/transport status,
page position, clearer focus and the existing cached native640×480 renderer.
Credits remain in help/dialogs and package/README notices; the main working area
has more space. One host-rendered main-frame image was inspected for essential new
layout validation; no device screenshot or repeated image analysis. Renderer
examples were regenerated from synthetic filenames for README.

Hold START+SELECT together800ms to open the existing reset confirmation directly.
X confirms, B/SELECT cancels. Defer individual taps until release so patches/help
do not open underneath the chord. Ignore repeats, fire once, rearm after both
release; cancel on focus loss and disable combined action in file/patch dialogs.
The SELECT→START fallback remains. No reset happens without X confirmation.

Apps label GrainMM, description “Granular synthesizer and effects”, original64×64
RGBA grain/wave icon. Root icon/config were exact-hash-checked and atomically
replaced, with old bytes retained in immutable releases/host archives. No launcher
logic, MainUI, services or outside-app settings changed. One source of truth still
provides90controls/settings10; no DSP/preset format change from alpha12/13.

## Verification

-22host/21ASan/UBSan/LSan cases passed, including overlap/short taps/repeats/tick
  wrap/cancellation/rearm and full SDL input probes. Final label-only change passed
  focused display/shortcut/editor checks. Independent legacy8/9 golden codec
  fixtures were added and passed a focused preset retest.
- Runtime5/tooling8/hardware8/source2 checks; formatting, analyzer and SDK ARM/ELF
  checks. Exact dependency list remains unchanged. Source/publication gates retained.
- Shared schema editor373→72lines before adding value-range display; codec179→74.
  No pointer offsets/type punning or metadata parsing in audio.
- New UI/shortcut code is UI-thread work. No extra audio buffers/voices or drawing/
  logging/allocations/locks in the callback. First64input events in local logs include
  timestamps/down/up for future physical diagnosis without more captures.

## Native application cases

Stock preload, SDL dummy video/audio,48kHz/stereo/1024frames; bounded isolated app
fixtures. Owner WAV/v9/v10 hashes preserved. Ten files/seven directories removed
individually. No fixture remains.

| Case | Result | Maximum | Late hints |
| --- | --- | --- | --- |
| New effect pages/save/migration | Expected values |23810us |2 |
| Reopen | Unchanged bytes |555us |0 |
| Sequential cancel/reopen/confirm/repeat |1reset, source fields preserved |23363us |2 |
| Actual held START+SELECT event sequence |1reset,4presentations/33requests |16819us |0 |
| Patch/waveform |1save/1load/2views, preparation10260us |17812us |1 |
| Alpha11 rollback | Loads preserved v9, leaves v10 |446us |0 |

Dummy input does not establish physical ergonomics/audio quality. Timing hints are
not XRUN counters. Alpha12's measured all-on DSP budget/outliers remain applicable;
UI improvements do not justify a speedup or zero-underrun claim.

## Storage and remaining acceptance

Only current alpha14 and verified alpha11 rollback remain, app3296KiB before owner
retry. Superseded alpha12/13 app files were checked against exact host-archive
hashes and individually removed; no user state or unrelated data was deleted.
Owner confirmed the UI works; reported incomplete-sounding reset and missing held
D-pad repeat. Alpha15 addresses those with reset scopes and accelerated editing.
Keep alpha11 LAST_GOOD until owner confirmation and closed-app log/state checks.
Long listening/stress, headphones/power/sleep/VNC-off and public native publication
remain separate gates. Full feature comparison/next batches: FEATURE_COMPARISON.md.
