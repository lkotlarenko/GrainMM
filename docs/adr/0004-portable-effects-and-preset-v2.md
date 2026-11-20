# ADR0004 — Portable effects graph and additive preset migration

Date:2026-11-18. Status: implemented, host/native-tested; owner confirmed effects controls, listening and save/reopen.

A full port needs a portable effect chain that can be tested without SDL. Add
Instrument (audio ownership/graph), Delay (DSP) and InstrumentParameters (complete
value snapshot). SDL owns backend negotiation and queues, not effect algorithms.
Fixed256-frame float scratch permits arbitrary backend blocks without callback
allocation. Existing S16 granular output remains available for regression/offline
users; normalized float rendering avoids quantizing between effect stages.

The first effect is an original two-second stereo delay with fractional linear
readout, smoothed time/feedback/mix/ping-pong/spread/filter coefficient, stable
one-pole feedback filtering and bounded feedback storage. Spread controls wet
stereo width; ping-pong crosses feedback. These semantics are documented equivalents,
not a claim of the unavailable native library's exact implementation. At48kHz
history occupies768016bytes. Logical history reset is O(1), with a valid-age guard
that prevents reading old storage after reset. Tiny feedback states are zeroed
below1e-20 to avoid subnormal CPU cost without requiring CPU flag changes.

A separate global20ms output fade suppresses delay tails on pause, focus loss
and exit. Clear history at the actual mute transition, even if the next call
immediately resumes. Do not clear a large ring in a real-time block. Preserving
musical tails during rack bypass is separate future routing work.

X/LeftShift toggles GRAIN/DELAY pages, with a key-edge guard. D-pad selects/edits
six real parameters per page. All values cross the same bounded SPSC queue.

Save12values in parameters.v2 with GrainMM2 header and exclusive
parameters.v2.pending transaction. Prefer v2 when present; otherwise read v1
and supply dry delay defaults. Leave v1 untouched so old verified builds can
still open. Corrupt v2 never silently falls back to v1. Failed pending writes
remain for recovery, and signals/power exits do not auto-save. This is a native
preset, not yet full GrainBrick JSON project compatibility.

Record actual callback frame-count min/max, produced frames and monotonic span
in bounded control-thread exit logs. Negotiated period alone does not establish
real backend pacing or underruns. Counters are32-bit lock-free values: produced
frames wrap after roughly24.8hours at48kHz; bounded tests must account for this.
