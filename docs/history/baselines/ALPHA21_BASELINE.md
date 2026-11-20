# Alpha21 — standalone generative DRONE

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. Owner clarified that a drone engine should be a self-running ambient
synth without WAV dependence. Alpha20's sample tool is renamed EVOLVE. Snapshot
and burgundy icon are owner-confirmed; alpha20 audio was not accepted by inference.

The new original source has four sustained voices, seeded harmonic progression,
smooth pitch/weight transitions, breathing/pan/pitch drift and filtered air. Six
controls steer LEVEL, ROOT, MOOD, EVOLVE SEC, TEXTURE and MOVEMENT. It runs through
the shared effects and suppresses the fallback tone while active. No sample copy,
new heavy effect bus or callback allocation/I/O/locks. See ADR0024 for bounds.

Settings14/120fields append6after114, preserving old1–13files. EVOLVE remains18,
DRONE19, STYLE UI-only20; ten source cards fit four columns. Other views retain
existing cards/cache/chrome. One relaxed unsigned telemetry value carries actual
evolution progress. No screenshots are required for this bounded layout change.

## Verification checkpoint

Focused model/DSP/UI/navigation/preset tests pass, including no-WAV stereo signal,
power/DC/bounds, deterministic split/reset output, automatic chord changes, pause,
allocation guards, shared-FX difference/reset and actual SDL no-WAV/save/reopen.
The test caught/fixed reset texture/movement initialization at silent apply.

Full30host tests passed143.53s before final air/pitch refinements; final focused6
passed4.16s. Final full sanitizer/ARM/source/native evidence and acceptance are
recorded in RESUME.md. Do not infer physical listening or stable deadlines from
these checks. Existing all-FX timing, headphones/long listening/power gates remain.
Native publication stays withheld; no reference code/assets were imported.


## Final verification and owner feedback

Final29sanitizer cases pass106.72s; independent267-file source build30pass137.11s.
Twenty native application cases and the native numerical/allocation suite pass.
No-WAV flow105callbacks/1024,worst5163us,late0,clips0; actual source telemetry is
nonzero, no grains/WAV required. Old13bytes and root owner files preserved; exact
fixtures removed. Current59217898da7b, verified18rollbackafe50220da16,3744KiB total.
Install194900bytes/repeat-identical. All11native default PCM checksums match18.

Native offline highest p95/worst us (11×93blocks/512):18default4878/16426.167,
21default5065.167/8869.167,texture5734.5/9754.834,texture+drone7665.833/11997.333.
Stock unmanaged DVFS/SSH/VNC; no controlled speedup or zeroXRUN claim. Legacy worst
23519us/1024 and migration23554us/late6 keep the timing investigation open.

Owner confirms standalone sound/functionality and snapshot/icon, but reports the
0.20maximum too quiet, generation dark/limited and scene too sparse. Alpha21 is
not promoted known-good on that feedback. Alpha22 should expand usable volume,
brighter harmonic options, event/pattern/seed steering and actual dynamic voice/
event feedback. No images were captured/analyzed.
