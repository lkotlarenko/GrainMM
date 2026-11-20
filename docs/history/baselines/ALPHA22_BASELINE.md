# Alpha22 — generative instrument usability

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. Owner confirms alpha21 works but is much too quiet/dark/sparse. Alpha22
adds normalized source level0–100%, wider register/eight moods, bounded melodic
voices, GENERATE steering and live target-note/activity display. Existing gains
remain unchanged. ADR0025 records architecture/migration/headroom. No screenshots.

## Level diagnosis

Private saved-patch host render: with FX RMS−30.13dBFS/peak−18.76; dry drone
−29.99/−21.02; sine oscillator at equal0.20 level−16.99/−13.98. No clipping. This
identifies source gain range, not a major FX attenuation. These are PCM, not SPL.
Maximum available level increases by about14dB; saved levels are preserved.

## Verification

Full30host tests pass149.38seconds. Full29ASan/UBSan/LSan pass114.29seconds outside
ptrace with leak checking retained. The initial sandbox sanitizer run hit the
known ptrace limitation; it is not counted as success. Focused7tests and expanded
actual SDL12-control/play/event/save/reopen flow pass. Runtime5/tooling8/hardware8/
source2 pass. SDK ARM hard-float build and dependency validation pass. Install ZIP
200873bytes; repeated packaging is byte-identical. Source/format/analyzer/reference
and matching-source validation are recorded at final checkpoint in RESUME.md.

Final22native app cases pass:16legacy, four standalone/schema13/reopen and two
copied-owner-schema14migration/reopen. Root owner WAV/v9–14hashes preserved. New
flow212callbacks/1024,worst5152us,late0,clips0,16updates,one actual melodic event.
Native allocation/determinism/headroom/pattern suite passes. Legacy worst37111us
and schema13migration24536us/late5 retain the scheduling issue; hints are not XRUNs.
Independent270-file source draft builds/passes30host tests144.82seconds.

Native offline11×93blocks/512per suite, highest p95/worst microseconds:

| Workload | p95 | Worst |
| --- | ---: | ---: |
| Verified18default |4813.667|10010.833|
| Alpha22default |4829.666|11809.500|
| Alpha22texture |6848.166|20596.167|
| Alpha22texture + drone/events |6370.000|8836.666|

All11default PCM checksums match18; all44workloads report software clips0. The
drone suite warms two virtual seconds before timing and enables maximal event
mix/rate; numbers are not a controlled comparison to the un-warmed texture suite.
Stock unmanaged DVFS, SSH and VNC; no overhead/speedup inference. Device currently
holds22and verified18only,3744KiB including owner data; prior21archived on host and
removed only after exact file/type/hash checks. Owner listening/UI acceptance is pending. No
controlled CPU improvement, acoustic loudness or zero-XRUN claim follows from host
or dummy audio tests. Backend XRUN count remains unavailable. All-FX scheduling
outliers, extended listening/headphones/power and VNC-off resources remain open.

Final review corrected the diagnostic minimum from6to8seconds to match the expanded
probe. The host no-WAV flow/runtime5 and sanitizer lifecycle pass afterward; short
requests are rejected before state access. This does not alter DSP/UI. Final staged
package awaits an idle device: preflight correctly refuses while owner is playing.
Current/queued immutable selectors are in RESUME.md. The superseded install/source
artifacts are retained privately on the host, not overwritten.
