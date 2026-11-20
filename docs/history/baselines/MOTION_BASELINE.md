> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Grain motion feature build — 2026-11-18

Candidate0.1.0-alpha.5 / `e5f93ff6d27b` is installed. Owner confirmed clean motion and save/reopen; LAST_GOOD now selects this build.
Verifiedalpha4 `deebcb9073ad` remains the rollback until the next candidate.
AppSHA256 `15dc23013caec152443e7dd50785d92e49bcc29b914019e8be0dcdbc5402f67c`.
ZIP75244bytes. Same SDK/runtime/input/display and no new shared dependency.

## Actual additions

Six MOTION controls: reverse probability, pitch variation in semitones, size/timing
variation, stereo pan spread and audio-clock sample scan. X cycles five real pages.
v5 saves30numeric values and source, preserving v1–v4. ADR0008 defines the behavior.
No borrowed DSP source, original binary dependency, placeholder control or exact
matching claim. EQ/reverb/delay/browser remain connected.

Host15CTest suites pass. Focused ASan/UBSan/LSan6suites (wave/audio replacement/
preset/PCM/granular/editor) pass; complete14suite sanitizer result belongs to alpha4.
Tooling8/hardware8, formatting, source/reference and ARM/ELF gates pass. Tests cover
negative positions and one-frame buffers at8/96kHz, randomized block invariance,
stereo output, reset/reproducibility, no allocation and v4 migration preservation.

## Native smoke and bounded performance

Three SDL-dummy tests with stock audio preload/search paths passed: v4→v5 migration
and browser replacement, all effects+motion loaded/resaved, and missing-file recovery
that preserves v5 bytes. Old v4 and the owner's WAV were hash-preserved. Temporary
fixtures were removed by exact name. All-effects+motion case:max8807us/1024,late0,
clipped0,9spawned/0drops. Migration case:max18568us,late-hint1; these are not XRUNs.

Offline benchmark: --seconds1/case,93blocks of512frames at48kHz, VNC2FPS+SSH,
app closed, CPU1200000kHz observed before/after (not locked). Full case includes
six EQ bands±6dB, delayMIX0.65/reverbMIX0.5 and all six motion controls nonzero.

| Case | p95 us/512 | Maximum us/512 | Clipped samples |
| --- | ---: | ---: | ---: |
| Default grains, full effects/motion | 2436.333 | 2683.666 | 0 |
| Dense grains, full effects/motion | 5086.500 | 20661.000 | 0 |
| Dense grains, wet delay/no motion | 2330.167 | 2554.833 | 0 |

Dense random scheduling hit the16voice limit:70spawned/9skipped; default case11/0.
The dense outlier exceeds the512frame arithmetic deadline. It may include OS
scheduling; do not claim WCET or zero underruns. Actual app negotiates1024frames.
Need owner listening and later a longer combined stress test before stable claims.
Reverb186912bytes + delay768016bytes; no additional heap for motion voices.

## Storage and continuation

App2464KiB before owner v5 save; only candidate and verified alpha4 rollback remain.
Alpha3 was hash-checked against its host tar before exact-file nonrecursive pruning.
No system/mixer/governor changes, new screenshots, unrelated-file writes or publication.
Next collect owner completed log/v5 state, then promote if confirmed. Continue
musical oscillator/resonator/routing features in the next version; keep this deployed
package immutable and retain only current + verified rollback on SD.

## Owner foreground acceptance

Owner confirmed motion works and the app is closed. Completed log:4922callbacks,
5040128frames,1024/callback,span104980ms,max18134us,late-hint0,723grains,0drops,
56updates,0software clips and normal exit. v5 contains reverse1,pitch jitter5,
size jitter0.9,pan0.6,scan0.45; older preset hashes are preserved. LAST_GOOD promoted
after matching current app checksum, completed log and saved v5. App2496KiB.
