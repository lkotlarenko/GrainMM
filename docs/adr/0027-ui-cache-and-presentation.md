# ADR0027 — Typed UI invalidation and bounded drawing work

2026-11-19. Status: implemented for alpha24; owner visual/audio/save check accepted.

The main renderer rebuilt an expensive scene on each selection move and formatted
strings even on idle polls. Its string key also omitted hidden parameters used by
waveform scenes. Adopt a typed 132-value control key with explicit source/view/chord
inputs. Allocate no new surface: retain the existing backdrop, reuse its scene on
selection-only updates, repaint only the two affected cards, and retain 20 Hz pacing.

Cache data stays in Display on the UI thread. The numeric key intentionally includes
all controls for correctness; no padding comparisons, pointer punning or audio-thread
reads. Values are formatted only when the scene/control state changes. Navigation
keys include visible active states. Explicit invalidation remains authoritative.

Increase value hierarchy and separate label/value/bar baselines. MIX is presented
as percent in shared metadata; the raw scalar, step, codec order and schema 16 do
not change. This avoids an audio or preset migration for a presentation decision.

The glyph font is original GrainMM work. Contiguous row spans are equivalent to
individual filled pixels, including rectangle rotation, but use fewer fill calls.
Numeric raster comparison verifies eight reference views. Group-local slider bounds
reuse the real bounded(group) implementation; reference tests verify equality to
bounding the entire instrument, including dependent sample limits.

No GPU, font library, FFT, extra visualizer DSP or new callback work is introduced.
Measured host request costs and limitations are in UI_QA_ALPHA24.md. Physical UI,
load and listening evidence remain separate from host/dummy success.
