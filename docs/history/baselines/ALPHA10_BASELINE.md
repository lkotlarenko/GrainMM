# Alpha10 development baseline

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-18. Version0.1.0-alpha.10; deployment cbf0bef287e0.
AppSHA256:58cfda2e90e85c1e6a5895eb039018f659ec57d99425ae96ee5858dbf984b70b.
Installation ZIP128503bytes; independently staged repeat is byte-identical.

## Scope

16 named patch slots with a bounded on-device name editor, explicit replacement
confirmation and transactional source loading. Cached256-bin stereo waveform
shows live non-destructive SAMPLE region edits without rescanning during audio.
Schema8 and66controls are unchanged. No DSP algorithm change in this batch.

CLI, application orchestration, display views, preset codec and patch storage
are separate modules. Each waveform is about2KiB; samples retain the existing
single bounded decoded buffer. New filesystem work remains on the control thread.
Current original work uses GPL-3.0-only; earlier MIT grants remain valid. Original
reference inputs and third-party licenses remain separate. Corresponding-source
archives and release publication checks are prepared; native source stays private.

## Host and static checks

- Full host19 CTest cases and sanitizer18 cases passed, including leak checking
  outside ptrace. After adding the negative patch-source probe, affected host3
  and sanitizer2 cases passed again.
- Runtime4, packaging/tooling8, hardware-tooling8 and source-archive2 passed.
- Clang analyzer passed new patch storage, menu, codec and waveform modules.
- Source/doc gates, formatting and440original reference hashes checked.
- SDK GCC8.3 ARM hard-float build and ELF/dependency checks passed. No new runtime
  shared-library dependency. library_test requires stdc++fs only with old GCC;
  do not run its host-style temporary-filesystem tests on the SD card.
- Actions lint passed locally; hosted workflows remain unrun/unpublished.
- An extracted reviewed source archive independently built and passed all19host
  cases, without access to the original payload, private guides or root build tree.

## Physical ARM, silent fixtures

Stock audio preload retained; SDL audio/video dummy at48kHz/stereo/1024frames.
Each test was bounded and used only an isolated App/GrainMM state fixture.

| Case | Result | Callback maximum | Late / clips |
| --- | --- | --- | --- |
| Patch naming/save/load, waveform/help, replacement cancel |1save,1load,2waveform views,1help; source preparation15204us |14482us |0 /0 |
| Reopen saved state | Unchanged committed bytes |436us |0 /0 |
| Patch with missing WAV | Old source still plays; failed patch/state untouched |14389us |0 /0 |
| Built-in synth/rack/help | Normal save and exit |4601us |0 /0 |

These are dummy-driver functional results, not evidence of physical sound quality.
Owner WAV/settings hashes matched before/after. All six known fixture files and
seven directories were individually checked/removed; no fixture remains.

Bounded offline512-frame workload: full tape/resonator/rhythm graph dense
p955040.833us, maximum6840.833us; matching checksum869. Normal full graph
p953329.666us,maximum3437.833us,checksum−3216. Other workload outliers reached
12897.166us. Dense grain drops9 are fixed-pool saturation, not backend underruns.
CPU frequency snapshots changed600→1200MHz; VNC remains enabled at owner-set2FPS.
No CPU policy was changed. This short run is not a controlled speed comparison.

## Physical controls and listening

A later normal dsp-driver session completed6809callbacks/1024frames over151150ms:
worst18848us,late0,clips0,1054grains/no drops,329updates,2waveform views,18toggles.
Its next paused reopen/exit completed218callbacks,474us maximum,1waveform view.
No new named-patch save/load occurred in those logs. The vendor audio initializer
logged a failed initial parameter setup then reopened48000Hz successfully; the
negotiated stream ran and closed normally. These logs do not prove audio quality.

Owner confirmed waveform trimming and named patch save/load on2026-11-18 and
left the app closed. Later sessions recorded2patch loads and2patch saves, normal
exit, software clips0 and no grain drops. One retained maximum was22797us/1024
(over the nominal21333us interval),late0; those are different diagnostics and do
not establish zero underruns. Alpha10 may now be retained as verified rollback. No new VNC
capture or image inspection was used. See RESUME.md for the current checkpoint.

## Storage and remaining gates

Only alpha10 and alpha9 releases remain, total app2944KiB after fixture cleanup.
Alpha8 files were matched against the host archive and pruned individually before
installing alpha10. Root launch.sh changed only its SPDX notice; script logic is
identical and alpha9 rollback remains available. No actively running executable
was overwritten. No system/mixer/network/clock/service changes occurred.

Long combined listening, repeated patch/source load/exit cycles, headphones,
power/sleep and VNC-off resources remain. Backend XRUN counters are unavailable;
late0 does not establish zero underruns. No stable/full-parity/public-release claim.
