# Alpha16 development record

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. Continues accepted alpha15; native publication remains withheld.

## Implemented

- Independent UI ordering: MIX bottom-right on eight wet/dry pages; output/makeup
  beside it where present. Original codec order unchanged.
- Audio-frame Q48 beat clock, phase-preserving BPM edits and transport restart.
- Generated16step slice source sharing the existing WAV and effects rack: forward,
  reverse, ping-pong/shuffle,2–16equal regions, quarter→32nd divisions, gate, four
  variations and reverse chance. GRAIN pitch/gain and SAMPLE trim apply.
- SLICE step/region feedback comes from audio telemetry, capped15FPS. Mode OFF
  restores grains. Bypassed grain-only controls are identified on affected pages.
- Settings11/96fields; absent-only1–10 migration preserves original files. New
  patches need alpha16+; older patch payloads remain readable.

Two slice voices in steady state; source crossfade temporarily allows16grain+2slice
voices and an extra2KiB scratch buffer. No extra sample copy, new dependency or
callback allocation/I/O/logging/locking. Timing must be measured on target.
Shared tempo is RHYTHM/BPM30–240; existing LFO is not phase-locked. Arbitrary manual
steps/boundaries, waveform playhead, metronome, snapshot and collision instrument
are not implemented by this batch. See ADR0019 and VIDEO_FEATURE_ROADMAP.md.

## Evidence

Early full host24case run passed before final reset/transport/display regressions.
Final gate results and hardware selector/acceptance are recorded in RESUME.md.
No new image inspections have been needed for the existing card layout/step strip.

## Final automated/native evidence

-24host/23ASan/UBSan/LSan cases passed (leak checks enabled outside ptrace).
- ARM numerical clock/slice/graph suite passed; SHA256:
 0572ea8e1c573986512d15cff18dff623f5d995028fd17d9b43a0fe16951aa89.
- Runtime5/tooling8/hardware8/source2, formatting, clang analyzer, source/doc/
  immutable440reference gates and SDK ARM/dependency checks passed.
- InstallationZIP162065bytes, repeat byte-identical. Native source stays private.
- CURRENT6275a2b5f2a4; appSHA256:
  2e19de1eff5920caedca2043d981511b69b120b3583123546a1618a1241b4adf.
- LAST_GOOD/PREVIOUS3cf7a9676d18 is owner-verified alpha15. Only these two releases
  remain,3360KiB total app/state. Alpha11's19files were individually checked against
  its host archive/type/hash inventory before pruning. No owner state was deleted.
- Eleven native SDLdummy/stock-preload cases pass: texture/save/migration/reopen,
  rack/modulation/factory reset, held edit/reopen, slice-v10migration/play/edit/
  save/reopen, patch/waveform and preserved-v9 alpha15 rollback. Owner WAV/v9/v10
  hashes preserved; fixtures individually removed. Slice case3triggers,7updates,
 34UIrequests/18presentations; worst11585us/1024,late0,clipped0. Owner confirmed clean sound,
  slicing/UI/consistent MIX and saving; final closed-app state/logs checked, LAST_GOOD promoted.

| Native dummy case | Worst callback us | Late hints |
| --- | --- | --- |
| Texture/save |18435 |1 |
| Reopen |630 |0 |
| Rack reset |19610 |3 |
| Modulation reset |22774 |2 |
| Factory paused |18593 |0 |
| Held edit |3824 |0 |
| Held edit reopen |656 |0 |
| Slice controls/save |11585 |0 |
| Slice reopen |924 |0 |
| Patch/waveform |22841 |3 |
| Alpha15 rollback |413 |0 |

## Bounded offline timing comparison

Stock VNC2FPS verified by process arguments; SSH connected. CPU observed800MHz
before and1200MHz after, no policy/mixer/service changes. One virtual second per
workload,93blocks of512frames,11workloads per suite. These are short offline
measurements under unmanaged DVFS, not end-to-end latency/XRUN or speedup evidence.

All11default GrainMM output checksums match verified alpha15. Old default suite
highest p956091.167us/max13086.833us; new default highest p955327.833us/max12154us.
Do not infer a speedup from this one changing-frequency pass. Dense default grain
workload still deliberately skips9new grains at the fixed16voice cap; this is not
an audio-buffer underrun.

New slice+all texture/effects suite highest p955554.500us/max20493.334us per512
frames, versus10666.7us nominal period. The worst all-on outlier remains a real
budget concern, even with clipped0/dropped0 in that suite. Longer/VNC-off listening
and profiling remain open. No zero-underrun/stable claim. Raw logs stay host-local.

## Owner acceptance

Owner reports slices/UI work, clean audio and saving. Final foreground report:
5365callbacks/1024frames,worst18746us,late0,858slice triggers,140updates,0clipped,
0dropped;2005UIrequests/644presentations,120529ms span. Three-second interaction
snapshot RSS/HWM6852KiB,3threads; not a stress or steady CPU measurement. Closed-app
schema11/hash checks passed; CURRENT/LAST_GOOD6275a2b5f2a4, PREVIOUSalpha15 retained.
Source/build archive remains local; no GitHub publication. No new device screenshots.
