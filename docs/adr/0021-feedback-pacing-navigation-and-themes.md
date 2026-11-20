# ADR0021 — Feedback fixes: pacing, navigation and color profiles

2026-11-19. Owner reports17working but laggy, balls hugging walls, tabs returning
to SOURCE before TEMPO, and requests selectable colors/readable burgundy default.

Main shoulders now traverse contiguous SOURCE/FX/TEMPO/STYLE groups independently
of stored page IDs. STYLE is a UI-only destination, not a fake audio parameter page.
A direct navigator remains available. Roundtrip/full-cycle tests cover every page
and group; older probes reach old semantic destinations through actual grouped
shoulder events. Help context survives traversing the STYLE view.

Drawing uses50ms dynamic deadlines and waits no more than20ms between main input
polls. This fixes40ms-poll/67ms-cap beat mismatch that could yield only12.5FPS.
Dynamic rendering now targets up to20FPS with the same cached backdrop; waveform
is never rebuilt per animation frame. This needs physical pacing/CPU/listening
checks; a cap alone is not proof of smoothness or audio reliability.

Wall restitution includes a .6+bounce minimum outgoing speed. This injects bounded
energy to keep the generative instrument moving rather than settling along walls.
Gravity/rotation/restitution still affect trajectories. A long virtual test checks
actual interior visits/motion. Voices/coefficient/energy bounds remain unchanged.

Six original themes: Burgundy default, Amber, Mint, Ice, Violet and Mono. STYLE
previews on up/down; A commits, B/X cancels, shoulders leave with saved colors.
Theme preferences are separate from audio presets/patches in theme.v1. Retained
procfs directory anchors, regular-file/size/symlink guards, exclusive pending file,
fsync/rename and preservation of failed transactions retain existing storage rules.
Bad preferences report/fall back without deleting the original. No per-frame I/O.
No sound format version bump; settings12 and all older files remain compatible.

## Vendor blit diagnostics and launcher follow-up

The real-device log showed SDL_CalculateBlitN on each cached SDL_BlitSurface call,
creating repeated SD log writes. Cache save/restore now copy only matching-size/
format surfaces, validate pixels/pitches and lock surfaces when SDL requires it.
Contiguous rows use one memcpy; padded surfaces copy only active row bytes. Both
surfaces retain the existing physical rotation. No SDL system library is changed.
Host display/sanitizer and bounded native probes cover the new path. This removes
our per-frame cache blit calls; it does not claim a measured CPU improvement.

Owner reports alpha18 substantially smoother and requests a matching launcher
icon. The original deterministic 64px icon now uses the default charcoal/burgundy/
ivory/mint colors with supersampled transparent edges. It is a fixed brand mark,
independent of the selected palette; choosing a theme does not rewrite launcher
assets. Root icon replacement is an explicitly reviewed app-only migration with
old/new hash checks and atomic rename. Older immutable releases stay intact.
