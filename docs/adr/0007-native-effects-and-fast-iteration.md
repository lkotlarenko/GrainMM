# ADR0007 — Working feature batches and native effects

Date:2026-11-18. Status: implemented; native smoke and owner listening/save/reopen passed.

The owner clarified the product goal: good musical results and useful GrainBrick
feature coverage, rather than bit-identical DSP or line-by-line translation.
Prioritize practical original native modules, larger working batches, a short
build/smoke gate and a verified rollback. Extended matching/soaks follow usable
builds. Device safety, existing data and callback constraints remain unchanged.

The first batch connects the portable EQ and adds an original compact stereo
parallel-comb/allpass reverb. Routing is fixed grain→EQ→delay→reverb→global fade/PCM.
This follows the reference's default EQ-before-delay/effects order without claiming
its configurable rack or exact reverb response. Reverb diffuse/mod/motion/shimmer
are not exposed. Each of four pages contains six actual working parameters.

## Control and audio ownership

InstrumentParameters contains numeric grain/delay/EQ-gain/reverb values. UI-thread
prepare_parameters calculates normalized EQ coefficients and reverb feedback/damping
coefficients. A fixed SPSC queue transfers complete trivially-copyable prepared
snapshots. The callback consumes at most7 and applies them without coefficient
design, allocation, file access or app locks. Offline/control convenience setters
may prepare parameters; the callback always calls Instrument::apply instead.

EQ uses its tested two-bank20ms crossfade and identity bypass. Six frequency/Q
pairs are fixed; only gain is exposed in this increment. Reverb owns8comb,4allpass
and2predelay rings, allocated once. Four independently delayed comb returns per
channel feed two unity-magnitude allpass stages. Decay feedback derives from an
approximate RT60 relationship; damping, fractional size/predelay, mix and width
slew per sample. Feedback/write headroom is bounded, tiny states are zeroed, and
history resets through valid-age counters instead of clearing large rings.
At48kHz, reverb rings use186912bytes. The dry path bypasses all reverb work.

A test found that a float smoother could stop short of full wet after its update
rounded to zero. Snap to target when the next value equals the current value, or
when the error is below1e-6. This removes the residual dry impulse at MIX1.
A/MENU fade and reset EQ/reverb history with delay; no stale tails on resume.
Final PCM remains hard-clamped. Count clipped samples in logs; this is not a
limiter or hardware clip/XRUN measurement. Lower grain gain before large EQ boosts.

## State and acceptance

Write24numeric fields and source selection to parameters.v4. Read v4→v3→v2→v1 only
on absence, never corruption. Older files and failed pending transactions remain
intact. Existing presets start EQ flat and reverb dry. The selected WAV is retained.

Host15/sanitizer14 suites, native preload migration/browser/missing-file smoke and
combined offline benchmark pass. See development/EFFECTS4_BASELINE.md. Owner confirmed clean effects/control/save/reopen; alpha4 is LAST_GOOD and
alpha3 is the preserved rollback. No screenshot
is needed: the existing six-row renderer supports the new page names/values.
