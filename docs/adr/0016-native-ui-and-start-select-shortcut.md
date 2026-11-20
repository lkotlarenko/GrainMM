# ADR0016 — Instrument UI and direct reset confirmation

Date:2026-11-18. Alpha14, following owner feedback on alpha13.

## Main instrument view

Use six two-column parameter cards, current source/transport state, page position
and normalized value bars. Parameter bounds come from the existing bounded model,
not another hand-maintained range table. Frequency bars use logarithmic display
spacing. The cached software surface is still640×480, with the proven physical180°
rectangle transform. Keep the25FPS ceiling and redraw only changed state. No new
waveform scan, animation, callback drawing or GPU/library dependency.

Move author credits from the working parameter area to help and retained dialogs;
README/CREDITS/package notices retain elly and ETRNLIAM attribution. Source/status,
control focus and values get the main working space. Palette/glyphs are original.
One host-rendered main-frame image was inspected for essential first layout review;
other matching renderer examples were regenerated without repeated inspection.

Apps label stays GrainMM. Description becomes “Granular synthesizer and effects”.
A deterministic64×64RGBA grain/wave mark replaces the previous debug-looking icon.
Transparent rounded corners allow the normal Onion background. No external artwork.
Bootstrap icon/config update is limited to exact hash-checked app-owned files.

## Hold START+SELECT to open confirmation

The owner found the sequential alpha13 shortcut too cumbersome and requested a
held START+SELECT gesture. Implement an800ms overlap hold that opens the same
explicit reset dialog; X confirms, B/SELECT cancels. Keep SELECT→START as a
sequential fallback. The gesture never resets audio without confirmation.

Defer normal START/SELECT tap actions until release: START taps open patches,
SELECT taps open help. Holding both suppresses those tap actions while waiting,
so menus do not open/close underneath the gesture. Fire once, rearm after both
release, ignore duplicate keydown, use wrap-safe SDL ticks. Focus loss cancels;
combos in browser/patch dialogs are disabled while individual tap actions remain.

A portable model tests overlap, repeats, short taps, tick wrap, cancellation and
rearming. Actual SDL event probes test held overlap and repeated key input through
the application, in addition to the sequential cancel/reopen/confirm path.
Bounded first64input events include down/up/timestamps in local logs so a physical
mapping issue can be diagnosed without more screenshots. No audio callback logging.
