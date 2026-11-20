# ADR0006 — Portable EQ with stable transitions

Date:2026-11-18. Status: accepted; integrated in alpha4 (see ADR0007).

Implement an original six-band parametric equalizer using the mathematical
transfer functions in Robert Bristow-Johnson's [Audio EQ Cookbook](https://www.w3.org/TR/audio-eq-cookbook/)
(W3C Working Group Note,2021-06-08). No upstream application/DSP source, external
implementation or runtime dependency is copied. Initial band frequencies/Q follow
the reference's behavioral defaults; this is not a full EQ V2.2 compatibility claim.

`design_equalizer` is control-thread work: clamp finite frequency/Q/gain, reject
invalid rate/kind and calculate six normalized sections. `Equalizer` receives
complete prepared coefficients. Its audio-owner set/process/reset methods have
fixed storage, no allocation, I/O, locks or trigonometric/exponential functions.
Future UI snapshots must prepare coefficients before entering the audio queue;
never call coefficient design from the callback or add inactive controls.

Support bell, low/high shelf and low/high pass sections. Shelves use Q damping,
not a separately exposed slope S. Gain-0 bell/shelves are exact identity sections.
Two independent stereo banks use direct-form-I history; each channel is isolated.
Default six-band gains are0dB. Alpha4 connects this module through prepared control snapshots.

## Transition and precision decisions

A first coefficient-ramp implementation failed the extreme-parameter sweep:
time-varying recursive coefficients produced excessive transient output despite
stable endpoints. Replace it with a20ms output crossfade between fixed stable
banks, clearing only the new bank. Coalesce updates arriving mid-transition into
one latest target. A newly arriving target supersedes stale pending values even
at an exact transition boundary. Identical snapshots preserve state. Pending
changes take at most the remaining fade plus the next20ms fade to complete.
A short crossfade can change phase/level during a filter switch; audible transition
acceptance remains necessary after integration.

Float coefficients also produced0.590600dB center error at25Hz/Q5/96kHz in the
boundary sweep. Keep coefficients and recursive history in double; input/output
stay float. The same coefficient-response sweep reports error below0.000001dB.
Native measurements show affordable standalone cost, but combined graph cost
still needs measurement. Do not sacrifice this accuracy without new evidence.
Cache identity bypass flags to avoid evaluating flat sections. Silence/subnormal
history is suppressed below1e-20; nonfinite input/output is contained. Strict
finite/Schur-pole validation rejects an invalid update without changing the target.
This is not a limiter or a guarantee against clipping from boosted bands.

## Acceptance and remaining work

Analytic corner/center/endpoint responses, real sine gain, identity, stereo
isolation, arbitrary blocks, reset, extreme settings, invalid input, latest-update
ordering and allocation checks pass on host and the physical ARM target. Sanitizers
pass on host. A bounded native benchmark measures flat, six active bells and
changing coefficients. See [EQ_BASELINE.md](../history/baselines/EQ_BASELINE.md).

Integration requirements, fulfilled in alpha4: define graph placement/headroom, add real six-band controls
and versioned persistence, predesign control snapshots, then benchmark combined
granular/delay/EQ and obtain physical listening/save/reopen confirmation. Additional
cutoff-contour stages and full upstream project mapping remain separate work.
