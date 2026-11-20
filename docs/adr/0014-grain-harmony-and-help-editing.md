# ADR0014 — Musical grain patterns and live parameter help

Date:2026-11-18. Implemented for alpha11; hardware acceptance tracked separately.

## Harmony within the existing grain budget

GrainBrick inspires broad musical feature coverage. Add an independently authored
per-grain chord/pattern selector instead of allocating a second engine or a pitch
shifter. This is not a claim of identical upstream harmony/ORBIX behavior.

HARMONY supplies transpose, chord, order, octave count, chord probability and
random detune. Prepare at most nine equal-tempered pitch ratios on the control
thread; the spawn path selects one with bounded integer work and an independent
fixed-seed RNG. It introduces no new sample buffers, grain voices or audio-thread
allocation/locks/I/O. Existing grains latch their pitch; pattern changes affect
new grains. Reset on stopped transport; unrelated parameter updates do not restart
phrases. Default output is identical to the previous renderer, verified numerically.

Keep existing linear interpolation. Large combined shifts can alias and are
explicitly documented; no claim of high-quality pitch shifting or key detection.
Benchmark both bypass/default and active harmony on the same full graph and
voice workloads. Existing16voice saturation remains a reported grain drop,
not a hardware underrun. Do not increase the pool speculatively.

## Persistent compatibility

Settings9 appends six fields,72total, and source selection. Only write parameters.v9;
load v8..v1 on absence of newer files, preserving older files and refusing corrupt
newer state. Named patch format1 accepts the previous embedded8 and current9
payload; new saves use9. Earlier apps cannot open newly saved9 patches. Retain
whole-state backups and explicit patch replacement confirmation.

## Help as a usable editing surface

Inspection found that parameter help advertised navigation it ignored. Keep the
D-pad and L/R working there; show the selected field/value and allow X to restore
only that field. Mark the normal pending snapshot/save path, retaining smoothing.
Help over a file/patch dialog disables editing and explains how to return. Cache
keys include edit availability, field and value. MENU always exits; held X cannot
repeat a reset or authorize patch replacement across modes. No new button chords.
