# GrainMM architecture

C++17 portable DSP with host SDL2 and verified Miyoo SDL1.2 adapters. Original
GrainMM implementation; no Python runtime or GrainBrick native libraries.

## Application and UI

`src/app/main.cpp` handles errors and dispatch. `options.cpp` parses/validates CLI
without SDL. `application.cpp` owns lifecycle, input/view state and orchestration.
`ParameterEditor` and `PatchMenu` are SDL-independent UI models. Source and patch
activation prepares the replacement before swapping ownership; failed loads leave
the previous instrument intact. All successful source changes start silent.

`display.cpp` owns software drawing/orientation/glyphs and cached parameter/help/
browser frames. `display_views.cpp` draws patch/name and waveform views. Waveform
min/max bins are built once in `model/waveform.cpp`; drawing and trim edits read
that fixed cache, never the sample in the audio callback. No GPU/FFT/font dependency.

## Audio ownership

`SdlAudio` owns negotiated format, callback state and Instrument. UI snapshots
are prepared outside audio and sent through a fixed SPSC queue; at most seven
messages are consumed per callback. Mute has an independent lock-free atomic.
No callback files, parsing, drawing, allocation or app locks. CloseAudio joins
before referenced samples/state are destroyed. UI swaps use the mixing lock.

Graph: Granular/Slices + OSC + Collision → Filter → Resonator → EQ → Tape → Crusher → Chorus →
Compressor → Rhythm → Delay → Reverb → mute fade/PCM. The quiet fallback tone joins after effects. DSP state is
bounded/preallocated; sample data is immutable. New grains latch region/window.
Backend counters report scheduling hints/work, not hardware XRUN counts.

## Storage

`preset_codec.cpp` validates/encodes all108controls and source independently of I/O.
`preset.cpp` handles ordinary versioned settings and absence-only legacy migration.
`patch_store.cpp` wraps that codec with names, fixed16slots and explicit replacement.
Retained directory descriptors, procfs anchors, type/size/path checks, exclusive
pending files, fsync/rename and recovery data protect state under Onion's preload.

WAV decoding already uses a fixed4KiB read buffer and a capped decoded vector;
unknown RIFF chunks are skipped, not copied into another full-file allocation.
No sample, patch or preview I/O occurs during a running callback. Immutable
package payload and mutable state remain inside App/GrainMM.

## Build, evidence and licensing

Exact source/package allowlists isolate original/private inputs. GPLv3 applies to
current reviewed GrainMM work; earlier MIT/third-party notices remain separate.
Public GPL releases need a matching reviewed source archive. Native source stays
private until explicitly authorized. ADRs explain decisions; current acceptance,
measurements and next work live in docs/development/RESUME.md. Historical baselines
are evidence for their versions, not current instructions.

## Grain harmony

A fixed nine-ratio prepared table is designed on the control thread. The granular
scheduler picks one ratio for each successfully spawned voice; existing voices
retain their increment. The fixed16voice pool and sample ownership do not change.
The independent pattern RNG preserves existing motion randomness when disabled.
No pitch exponentials or allocation occur in the new per-grain selector. Schema9
adds six fields; strict autosave migration preserves v1–v8 and named patches accept
both their previous v8 payload and the new v9 payload.

## Alpha12 rack and shared control schema

Add filter before resonance, crusher/chorus before compression. All settle to exact
dry bypass, use prepared coefficients and fixed storage. Source/grain voice budget
is unchanged. parameter_schema.cpp provides typed field accessors, UI labels/steps/
choices and historical codec prefix counts. grainmm_parameters depends on portable
DSP; DSP does not depend on UI metadata. No offsets/type punning are used.

Hold START+SELECT to open a three-scope reset confirmation (SELECT→START fallback).
The UI thread drains old queued snapshots and clears rack history with joined
callback ownership. Rack-only preserves sources; modulation also clears grain and
synth shaping; all-controls restores defaults and pauses. Sample ownership/paths
and stored patches remain independent. MENU saves edited ordinary settings.

DirectionalRepeat owns bounded UI timing, suppressing OS repeats. It emits at most
one repeat per frame, accelerates scalar edits only, and cancels across views/focus
loss. The callback receives ordinary prepared snapshots, never key-repeat logic.

## Audio-clock slice source

FrameClock is audio-owned Q48 beat phase; prepared tempo retains phase on edits,
transport starts at zero. SlicePlayer consumes a block-anchor copy; master advances
once per frame. Fixed prepared16step patterns share immutable sample ownership.
Two slice voices with edge/retrigger ramps feed the same rack;20ms grain/slice
crossfade is bounded at18temporary voices and one additional2KiB scratch array.
Disabled steady mode skips slice rendering. No duplicate sample or new dependency.
Schema11 appends six fields; previous1–10 remain intact. UI reads packed lock-free
step/region telemetry, never mutable voice state. Existing LFO shares BPM but retains
its independent phase. ADR0019 defines limitations and source-control behavior.

## Alpha17 instrument interface and collision source

The UI now separates primitives/chrome (display.cpp), instrument scenes/cache
(display_instrument.cpp) and modal/module views (display_views.cpp). PageNavigator
owns SOURCE/FX/TEMPO grouping independently of saved order. X selects modules;
A confirms there and retains main transport behavior. One same-format SDL backdrop
caches labels, waveform and controls; dynamic frames blit then draw bounded live
telemetry. Text stays inside640×480, dynamic updates <=15FPS, controls <=25FPS.
Latest-block stereo peaks decay visually; they are not calibrated true-peak meters.

CollisionSynth owns120Hz audio-frame hexagon-wall physics and eight two-mode
resonator voices. Six fixed ball states, prepared stable note coefficients and
quietest-voice replacement bound work; no sample ownership or rendering dependency.
It layers before the existing shared FX rack. Maximum logical source voices are
16grains+8collision voices, or2slices+8collision voices; crossfade can briefly use
16+2+8. UI receives packed lock-free poses/hits/levels/grain activity, never reads
mutable voice state. Schema12 appends12fields, preserving all1–11 files. ADR0020
records scope/visual limits and keeps previous timing outliers open.

Alpha18 separates grouped visual traversal from storage IDs and adds STYLE as a
UI-only destination.50ms animation deadlines/<=20ms input waits avoid cadence
aliasing. Theme preferences have separate guarded atomic storage; no UI values
enter the callback/preset payload. Modal transport state stays accurate. Wall
rebound energy remains bounded and deliberately sustains the generative source.
See ADR0021; current caps/default palette are documented in RESUME.md.


Temporary sound recall (ADR0022) is control-thread owned: one bounded parameter
copy plus a source generation, no sample pointer or disk transaction. Successful
source replacement advances its generation. Recall refuses stale/empty slots,
uses the existing prepared queue and preserves transport/effect history. SELECT
modifier consumption is portable/tested separately from SDL event dispatch.


DRONE (ADR0023) is a separately seeded audio-owned modulator in the granular
source. Wave/drift/blend targets refresh every64frames, slew smoothly and latch
into new grain position/pitch/spread without mutating the base parameter model.
Off is numerically transparent; hold/pause freeze phase. Fixed voices/sample
ownership remain. Settings13 append6fields; UI-only STYLE is page19. Waveform
and existing grain telemetry provide feedback without another visualization bus.


Alpha21 separates EVOLVE(sample motion) from DRONE(standalone generative source).
DroneSynth owns four voice phases, prepared harmonic steps, deterministic event/air
streams and original8KiB tables. Smooth harmonic/weight/timbre transitions and
rate-scaled air processing are bounded, allocation-free and sample-independent.
It enters the existing shared rack; target level/fade suppress fallback tone.
One lock-free value exposes chord/cycle progress, not mutable audio state. Settings14
append6fields after114; SOURCE uses4columns for10cards. See ADR0024.
