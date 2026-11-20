# GrainMM architecture

Current source: 1.0.0-alpha.1 synthesis/groove candidate. Alpha25 remains the
accepted device checkpoint; new feature acceptance is separate. See [the handoff](docs/development/RESUME.md) for evidence.
GrainMM is original C++17 work; it uses no GrainBrick Python runtime or DSP binaries.

## Modules

| Directory | Responsibility |
| --- | --- |
| `src/app` | CLI validation, lifecycle and composition |
| `src/audio` | Immutable samples, WAV codecs and instrument graph |
| `src/dsp` | Portable, bounded synthesis and effects |
| `src/model` | Parameter schema, presets, patch menus, waveform cache and drone scenes |
| `src/platform` | SDL audio, drawing, input and target adapters |
| `include/grainmm` | Ownership contracts and interfaces |
| `tests`, `benchmarks` | Numerical/lifecycle checks and bounded measurements |

Host uses SDL2; Miyoo uses the verified SDK SDL1.2 adapter. Software rendering is
640×480 with target rectangle rotation. No GPU, FFT or external font dependency.

## Audio graph and ownership

Grains/slices + dual oscillators (A with wavetable/sub color) + collision instrument + generative drone feed:

`Filter → Resonator → EQ → Tape → Crusher → Chorus → Compressor → Rhythm → Delay → Reverb → OUTPUT → transport fade → PCM`

The diagnostic tone joins after the FX rack and before OUTPUT. OUTPUT defaults to
neutral gain with limiting off. It provides gain, linked sample-peak limiting,
ceiling/release, balance and mono; it is not a true-peak/lookahead limiter.

Samples remain immutable while referenced. Source replacement prepares the next
sample/instrument before switching ownership and starts paused. Failed loads keep
the previous sound. SDL audio closes before referenced state is destroyed.

The callback performs no allocation, files, parsing, logging, drawing or application
locks. Complete prepared snapshots use a fixed SPSC queue, with at most seven
messages consumed per callback. Mute is independent. Expensive coefficient/pitch
design stays on the control thread. DSP works in fixed 256-frame scratch chunks.

Granular playback has 16 voices; slices have two, with a bounded transition between
them. The collision source has six balls and eight two-mode resonator voices.
DRONE has four sustained and four short event voices. Sources share the same rack;
there are no duplicated sample or reverb buffers. EVOLVE modulates WAV grains;
DRONE generates sound independently. The frame clock drives slice sequencing.

## Interface and state

Typed parameter metadata defines 25 stored pages, 150 scalar controls and schema18.
Storage order is append-only; grouped navigation has independent display order.
SOURCE has 14 destinations in two banks, FX has nine, TEMPO has two and TOOLS
has export, patches, master output and colors. Colors are UI-only. Dynamic drawing is capped at 20 Hz with cached surfaces and bounded
input waits. Telemetry is value-only and lock-free; UI never reads mutable voices.

Waveform bins are computed once on loading. Scenes apply only drone generation,
preserving level, rack, other sources and transport. Temporary snapshots hold
parameters plus source generation; they neither copy samples nor write files.
Six color preferences are stored separately from sound settings.

Preset codecs validate independently of I/O. Rooted paths, directory descriptors,
procfs anchors, type/size checks, exclusive pending files and atomic replacement
protect settings, 16 named patches and failed-save recovery. Older settings1–17
remain intact; schema18 is written only after edits and normal exit.

## Evidence and development boundaries

The target is Miyoo Mini Plus / OnionOS 4.3.1-1, ARMv7 hard-float. Host/SDK success
does not establish audible hardware behavior. Callback lateness is not an XRUN.
Longer stress, power/sleep, headphones and VNC-off resource checks remain open.
Native publication requires separate authorization and matching reviewed source.

See [ADRs](docs/adr/0001-platform-and-runtime.md), [quality policy](docs/CODE_QUALITY.md),
[testing](docs/TESTING.md) and [the historical architecture](docs/history/ARCHITECTURE_THROUGH_ALPHA21.md).

## UI QA cache and metadata

The renderer uses a typed150-value cache key plus explicit source/view/chord inputs.
Selection-only updates reuse the scene and repaint two cards; hidden parameter
changes correctly invalidate scenes. Original glyphs are drawn as row spans with
identical raster output. Slider metadata can query real group-local bounds, avoiding
whole-instrument bounding per field; reference tests preserve dependent limits.
MIX uses percentage presentation without changing DSP or stored values. No new
full-screen cache, callback work or visualizer DSP was added. See [ADR0027](docs/adr/0027-ui-cache-and-presentation.md) and the archived UI baseline.


## Composition and export

`SlicePattern` is a fixed instrument value, prepared into steps/cuts on the control
thread and carried by the existing bounded queue. The dedicated composition
controller handles pattern/export screens without adding a filesystem path to audio.
Autosave, named patches and snapshots carry the full pattern; schema18 stores
150 scalar values and separately versions the pattern payload.

`InstrumentExport` pauses live playback, borrows the immutable sample and owns a
fresh complete instrument graph and exclusive writer. It renders bounded chunks
between UI event polls. Outputs stay in `state/exports`; cancellation checks the
actual writer identity and never deletes completed or replaced files. See
[ADR 0028](docs/adr/0028-composition-patterns-and-offline-export.md).


## Synthesis and groove milestone

Oscillator A supports generated table morph/scan plus a separate sub level; B
reuses the bounded oscillator with independent pitch/timbre. Shared immutable
harmonic tables are generated during construction, not in rendering. Band blends
reduce high partials with pitch. Levels, pitches and color transitions smooth.

Slice swing uses the Q48 clock, independent of UI/render chunk size. Odd steps
are delayed while pairs retain duration. Phrase length/traversal/rotation maps
transport steps to the unchanged16-step payload. Accents attenuate offbeats.
See [ADR0030](docs/adr/0030-synthesis-groove-and-major-milestone.md) for contracts
and [1.0 validation](docs/development/V1_VALIDATION.md) for actual evidence.
