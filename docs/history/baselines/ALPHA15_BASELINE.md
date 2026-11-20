# Alpha15 development record

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-18. Controls/reset follow-up to alpha14; native source remains private.

## Changes and limits

Owner confirmed the new UI works, requested held arrows and reported incomplete
reset. Rack-only intentionally preserved motion/harmony/synth modulation; the owner
chose reset options rather than an unconditional wider reset. The three-scope
confirmation now defaults to effects plus modulation; all-controls resets every
value and pauses. WAV selection/ownership and stored patches remain independent.
A PLAY/PAUSE works while choosing. Processing pages show parameter-based ACTIVE/
BYPASS; this is not a detector of audible tails or the whole source signal.

Held arrows repeat after320ms; scalar edits every60ms accelerate5× at1.5s and10×
at3s. Navigation repeats140ms without acceleration. Ignore OS duplicates, at most
one generated event/frame, release stops, modal changes cancel, focus loss clears.
Short START/SELECT taps remain deferred; interrupted overlap restarts the800ms
continuous hold. No callback DSP, preset format or dependency changes from alpha14.

## Verification record

Numerical reset tests distinguish source preservation, modulation clearing and all
sound defaults. SDL paths exercise rack-only, held-default-modulation, paused all-
controls reset and accelerated editing/save, plus previous patch/migration coverage.
An initial new Python assertion expected an unquoted filename; the codec correctly
stores quoted paths. The assertion was corrected; earlier failed logs are retained
and must not be represented as acceptance. Final evidence is recorded in RESUME.md.

Dummy/host input does not establish physical repeat ergonomics or audio quality.
Alpha12/14 timing outliers remain documented; no new DSP speedup/XRUN claim.
No additional screenshot inspection was needed for this follow-up.

## Final candidate evidence

CURRENT3cf7a9676d18; appSHA2566f5c73f702572230cdb4b3fb7cf408482ac7c28ec288aa7c980ee2a041641d3a.
ZIP152977bytes, repeat byte-identical. Host23/sanitizer22 passed; leak checks enabled.
Native nine cases passed, including held SIZE reaching250 and24control updates;
reset logs distinguish scope0playing1/scope1playing1/scope2playing0. Owner WAV/v9/v10
hashes unchanged. All fixture files removed individually; only current plus verified
alpha11 rollback retained. Owner confirmed repeat/reset choices work, audio clean and settings saved; closed-app
logs/state checked, alpha15 marked LAST_GOOD with alpha11 explicit rollback.

| Native dummy case | Worst callback us | Late hints |
| --- | --- | --- |
| Texture/save/migrate |16341 |0 |
| Reopen |498 |0 |
| Rack reset |22347 |3 |
| Modulation reset |12966 |0 |
| All sound paused |14397 |0 |
| Held edit |4062 |0 |
| Held edit reopen |449 |0 |
| Patch/waveform |18612 |0 |
| Alpha11 rollback read |614 |0 |

Stock VNC2FPS and SSH were available; no CPU policy change. These short dummy
runs are neither audible stress tests nor hardware XRUN measurements.

Owner foreground log:7405callbacks/1024frames, worst23058us, late0,2454grains/
0dropped,315updates,0clipped;3150UIrequests/416presentations. No hardware XRUN
counter; clean short listening does not establish long-term stability.
Independent extracted source built and passed23host cases; source packaging repeats
byte-identically. No native source, binary, tag or workflow was published.
