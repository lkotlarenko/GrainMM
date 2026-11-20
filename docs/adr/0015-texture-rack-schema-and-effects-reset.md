# ADR0015 — Texture rack, shared schema and guarded dry reset

Date:2026-11-18. Alpha12 implementation, amended for alpha13 after physical feedback; physical evidence recorded separately.

## DSP

Add original bitcrusher, state-variable filter and stereo chorus. No external DSP
code/assets or new runtime dependencies. Prepare exponentials/tangent/coefficients
outside audio; smooth controls/mode weights. Bypass is exact when mix settles to0.
Filter uses double integrator state for low-cutoff precision. Chorus is a fixed
32KiB stereo4096frame delay with linear reads; sample-rate range8–96kHz and read
bounds are explicit. Crusher's sample hold deliberately aliases; it is a creative
reduction effect, not a resampler. High resonance/drive can still clip.

Graph: sources → filter → resonator → EQ → tape → crusher → chorus → compressor
→ rhythm → delay → reverb → mute/PCM. Dry fallback tone still bypasses the rack.
Measure all-on costs without increasing the16grain voice budget or sample copies.

## Fast feature integration

A typed90field schema shares label/step/read/write/choice metadata between the
editor and codec. Historical formats are fixed prefixes; source stays separate.
No raw offsets, untyped pointers or callback parsing. New independent parameter
metadata library depends on portable DSP; DSP does not depend on UI/schema.
Settings10 appends18fields and preserves v1–v9. Patch wrapper1 accepts old/new
payload versions; new patches require an app that understands their payload.

## Reset shortcut

The alpha12 simultaneous SELECT+X hold passed synthetic events but the owner
reported it unreliable on physical controls. Replace it in alpha13 with a deliberate
sequential action: SELECT opens help, START opens a dedicated reset confirmation,
X confirms; B/SELECT cancels. X in normal help still resets one field. Held/repeated
X cannot confirm twice, focus loss cancels, and MENU always exits. No simultaneous
button holds or firmware shortcuts are required. The application probe tests a
cancelled confirmation, reopening, one reset and duplicate X-down suppression.

Reset all processing-rack controls to dry defaults; preserve WAV/source, oscillator,
grain/trim/harmony and patches. Mute/pause, drain stale prepared commands, apply
reset values and clear FX history under joined callback ownership, then resume
prior play/pause state. Filesystem work is not introduced into audio. A short mute
is intentional so old delay/reverb tails cannot contaminate the dry comparison.
Normal MENU saves edited ordinary settings; saved patches are unchanged.
