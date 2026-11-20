# Alpha20 — evolving grain drones

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. DRONE adds periodic/seeded drift/blended position, pitch and spread
movement to granular WAV playback. Scan hold freezes movement, not audio; base
settings remain separate. Fixed16grain voices, shared immutable sample and
allocation-free audio remain. Slice/oscillator/collision sources are unaffected.
See ADR0023 and PARAMETERS.md for exact controls and limits.

Settings13/114fields append six after the unchanged108prefix; old1–12files remain
untouched. Snapshots/patches include drone state; new patches require alpha20+.
Nine SOURCE cards and twenty total UI destinations retain contiguous group order.
STYLE is UI-only19, DRONE is stored18. Fixed collision view's old >=16assumption.

## Verification

-29host tests pass132.77s;28ASan/UBSan/LSan pass99.18s outside ptrace, with leaks
  enabled. Final STYLE caption passed focused host/sanitizer UI/drone/navigation.
- Periodic reference, deterministic bounded drift, hold/pause/disable settling,
  PCM whole/split/reset equality and allocation guards pass at8/48/96kHz.
  Mode OFF is identical even with other drone controls nondefault. Fixed voice
  scheduling/counts stay equal; modulation changes actual PCM when enabled.
- Actual SDL grouped-key flow visits DRONE, edits all controls, plays/holds/resumes,
  opens help, stops/saves/reopens; base position remains0.5. Old settings12 bytes
  are preserved through13migration and new controls roundtrip.
- All11default host offline PCM checksums match alpha19. SDK ARM ELF/dependency
  validation and runtime/tooling/hardware/source gates are recorded in RESUME.md.
- Package186849bytes, repeat byte-identical; matching source is separately reviewed.
  Native deployment, measurements and owner listening are recorded in RESUME.md.

No screenshots required: existing cards/waveform/one base marker, bounded text and
all-page/cache tests. No change to mixer/governor/services/system libraries. Older
callback-duration outliers, headphones, power/sleep, longer listening and VNC-off
comparisons remain. Disabled bit-identical output does not prove zero target cost;
callback lateness is not a backend XRUN counter. Native publication is withheld.


## Native evidence

Candidate d459de91b6b0/appSHA23c6b5d6d9cd613bf0b33f9a14fb8a3a42503cc8e3a51f0bc091397d29e1510b.
Accepted18afe50220da16remains rollback pending owner listening/gesture checks.
Sixteen bounded stock-preload/dummy cases pass; original WAV/v9/v10/v11/v12 hashes
preserved, exact fixtures removed. New scene103callbacks/1024,worst28924us,late2,
35grains/no drops/clips,10updates,43presentations/12layouts. Batch worst29329us.
The standalone native numerical/allocation suite also passes; temporary executable
hash-checked before/after and removed. No audio was emitted by these tests.

All11default native checksums match18. Each suite11workloads×93blocks/512frames:

| Workload | Highest p95 µs | Worst µs |
| --- | ---: | ---: |
| Alpha18 default |4924|10160.334|
| Alpha20 default |4850|10123.333|
| Alpha20 texture |5795.833|11617.333|
| Alpha20 texture + drone |6755|11185.5|

Stock unmanaged DVFS, SSH and existing VNC; no system settings changed. These
sequential short runs do not isolate overhead/speedup. Some all-FX blocks exceed
the nominal512frame deadline and native dummy callbacks retain scheduling hints;
keep the existing timing investigation open, not a zero-XRUN/stable claim.
Independent261-file source draft builds/passes29host tests127.75s; final source
refreshes documentation only. No screenshots/image analysis. Current SD inventory
and owner acceptance are recorded in RESUME.md/private journal.


## Owner feedback and corrected direction

Snapshot/icon work. Owner says the visible20page is not the intended drone engine:
it should be an automated standalone ambient synthesizer, steerable and routed
through the existing effects. The supplied M5Burner3NRPTY reference matches
MIASMORA via its author README. No source/assets copied. Keep this useful sample
modulation under EVOLVE; implement a separate original generative DRONE source.
No clean-audio/save acceptance or LAST_GOOD promotion is inferred for20.
