# Alpha24 — UI QA and render optimization

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. GrainMM by elly (@lkotlarenko). UI pass on the existing instrument;
no new DSP engine, controls, sample buffers or operating-system changes.

## Findings and changes

- Labels and values on parameter cards were crowded. Cards now have separated
  label/value baselines, larger values, stronger focus and consistent progress bars.
- Wet/dry controls consistently show MIX %; raw values, steps, storage and DSP units
  remain unchanged. A displayed 30 is the existing stored 0.30, not additional gain.
- Navigation shows ACTIVE/BYPASS for effects and ENABLED/LEVEL ZERO/OFF for independent
  sources. State changes invalidate the navigation cache.
- The built-in source caption now identifies an enabled collision instrument.
- Main scene caching previously omitted non-visible parameters. Sample trim changed
  while on HARMONY could leave a stale region. Typed keys cover all 132 values,
  source/view inputs and drone chord state; hidden edits now rebuild correctly.
- Selection changes reuse the scene and redraw only the old/new parameter cards.
- Glyph drawing merges contiguous horizontal bitmap pixels. Eight reference views
  are byte-identical before/after that optimization, including the modal views.
- Slider ranges use typed group-local bounds instead of bounding the entire
  instrument for each field. Reference tests compare all 132 controls at different
  sample-region states; fallback metadata retains the full reference path.

No extra full-screen cache was added. The 20 Hz animation cap, direct surface copy,
640×480 geometry and target 180° rotation remain. Changes to DSP controls still use
the same prepared queue. The audio callback does not format or draw UI data.

## Visual and numerical QA

Only two contact sheets were inspected: initial and revised main/navigation/style
views. No device screenshots were taken. Automated bounds checks cover all 22 main
pages across six palettes at both parameter extremes, plus dialogs/help. Glyph
raster comparisons are numeric; transient captures are archived privately.

Regression checks cover selection scene reuse, hidden trim invalidation, explicit
invalidation, navigation status updates and exact range equivalence. Existing
wave/control/save/reopen tests verify unchanged raw parameter contracts.

## Host measurement

Same host, SDL dummy video/audio, C++17/O2 renderer, identical 2,000-request scenarios.
The portable libraries use the retained host-debug build in both runs. This is a
UI microbenchmark, not an end-to-end audio benchmark or target framerate guarantee.

| Scenario | Before us/request | After us/request | Observation |
| --- | ---: | ---: | --- |
| Idle |2.555|1.780|About30% less request work |
| Selection |515.325|131.048|About75% less work; one scene instead of 2,000 |
| Value editing |516.367|516.601|Approximately unchanged, with larger value rendering |

The comparison is one recorded pair, not a controlled frequency/thermal experiment.
Do not infer a universal speedup or multiply these numbers into audio CPU savings.
Editing a graph parameter still rebuilds its scene. Source selection and glyph
optimization benefit other views too, but only the scenarios above are measured.

Reproduce a bounded host run after building:

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy build/host-debug/ui_benchmark --requests 2000
```

The tool requires dummy video and accepts 1–2,000 requests. Use a much smaller workload
for native tests. Final test/ARM/native evidence and candidate identity are in
RESUME.md; prior version baselines are historical. Callback lateness is not XRUNs.

## Native checkpoint

26 silent app cases and two numerical suites pass, including source/control/reset/
patch/save/reopen and schema 13–15 migration. Existing owner WAV/settings hashes are
preserved; temporary fixtures/binaries were individually removed. One accidental
overlap attempt was refused by the existing running-app guard; the subsequent
sequential suite passed. No screenshots or audio playback were forced on the owner.

Three 100-request dummy-video scenarios measured idle 91.038 us/request (including the
first scene), selection 4,002.622 us and editing 4,733.685 us. This is not real framebuffer
cost, worst-case frame latency, VNC-off profiling or audio/UI concurrency clearance.

All 11 default native DSP checksums match verified alpha18. Highest p95/worst us per
512 frames: alpha18 default 4,883.333 / 11,305.667; alpha24 default 5,088.334 / 18,100.5;
alpha24 texture 6,111.5 / 16,341.5; texture + OUTPUT 5,858.667 / 19,646.166. All 44 offline cases report
software clips 0. Stock unmanaged conditions/SSH/VNC; scheduling spikes remain open.
No universal DSP speedup or zero-XRUN claim is supported.

Device keeps alpha24 plus verified alpha18 only: 8,384 KiB including owner WAVs/state.
The owner confirmed readability, responsiveness, clean listening and save/reopen.

Native app-case worst callback 26,783 us and 18 late hints across different cases retain
the existing scheduling investigation. They are not measured hardware XRUNs.

## Owner acceptance

Owner reports readable/responsive UI, clean sound and saved settings. Closed process,
installed binary hash and schema 16 save verified. Physical session log: 10,002 callbacks
of1024frames,span223550ms,worst28577us,late0,softwareclips 0;491slices,776collision
impacts, 73 drone events and 242 control updates. User listening is distinct from the
software counters; no hardware zero-XRUN claim. LAST_GOOD is alpha24, PREVIOUS remains alpha18.
Longer stress/headphones/power/VNC-off gates remain open.
