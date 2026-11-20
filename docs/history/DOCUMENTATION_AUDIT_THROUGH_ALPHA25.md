# Documentation audit — historical through alpha25

Reviewed every repository Markdown document (including root project guidance),
package notices, license scope/texts and release/build metadata. Private guides
and original reference payload were not changed. The current index is docs/README.md.

## Corrections

- Replaced stale architecture, testing, target-platform and release/roadmap guidance.
- Moved initial test/platform/device/granular reports to history, preserving their
  measurements and hashes; fixed relative links after relocation.
- Marked versioned baseline evidence historical; retained earlier MIT release notes
  because those grants and version-specific statements remain valid.
- Added the current PARAMETERS guide and kept GRANULAR_PROTOTYPE as a compatibility
  link instead of breaking older references.
- Updated current GPL language while retaining third-party MIT/LGPL distinctions.
- Added a local documentation path/heading gate; no private guide reads/network calls.
- Public/native publication boundaries remain explicit. Hosted CI is not claimed run.

Verbatim GPLv3 text: SPDX license-list-data text/GPL-3.0-only.txt,
Git blob f6cdd22a6c1fbc887e08a215cb4beb3c47048041,34674bytes.
SHA256: fb981668c18a279e285fc4d83fba1e836cc84dd4daa73c9697d3cfd2d8aca6e0.
Earlier LICENSES/GrainMM-MIT.txt remains unchanged.

## Inventory

This table classifies the reviewed documents; historical statements are not
rewritten into present-day claims. Re-run check_docs.py after future changes.

| Document | Role |
| --- | --- |
| `docs/ABOUT_GRAINMM.md` | Current guidance |
| `docs/AUDIT.md` | Historical evidence |
| `docs/BUILDING.md` | Current guidance |
| `docs/CODE_QUALITY.md` | Current guidance |
| `docs/CREDITS.md` | Current guidance |
| `docs/DEVICE_TEST.md` | Current guidance |
| `docs/GRANULAR_PROTOTYPE.md` | Current guidance |
| `docs/INSTALLATION.md` | Current guidance |
| `docs/LEGAL_AND_PROVENANCE.md` | Current guidance |
| `docs/PARAMETERS.md` | Current guidance |
| `docs/PORTING_PROGRESS.md` | Current guidance |
| `docs/README.md` | Current guidance |
| `docs/development/REMOTE_DEBUGGING.md` | Current guidance |
| `docs/TARGET_PLATFORM.md` | Current guidance |
| `docs/TESTING.md` | Current guidance |
| `docs/TROUBLESHOOTING.md` | Current guidance |
| `docs/UPSTREAM.md` | Current guidance |
| `docs/adr/0001-platform-and-runtime.md` | Dated architecture decision |
| `docs/adr/0002-sample-and-granular.md` | Dated architecture decision |
| `docs/adr/0003-preset-storage-and-onion-preload.md` | Dated architecture decision |
| `docs/adr/0004-portable-effects-and-preset-v2.md` | Dated architecture decision |
| `docs/adr/0005-rooted-sample-browser.md` | Dated architecture decision |
| `docs/adr/0006-portable-equalizer.md` | Dated architecture decision |
| `docs/adr/0007-native-effects-and-fast-iteration.md` | Dated architecture decision |
| `docs/adr/0008-grain-motion.md` | Dated architecture decision |
| `docs/adr/0009-built-in-synth-and-compressor.md` | Dated architecture decision |
| `docs/adr/0010-tape-coloration.md` | Dated architecture decision |
| `docs/adr/0011-alpha9-rack-and-sample-tools.md` | Dated architecture decision |
| `docs/adr/0012-gplv3-and-source-distribution.md` | Dated architecture decision |
| `docs/adr/0013-patches-and-waveform-workflow.md` | Dated architecture decision |
| `docs/adr/0014-grain-harmony-and-help-editing.md` | Dated architecture decision |
| `docs/adr/0015-texture-rack-schema-and-effects-reset.md` | Dated architecture decision |
| `docs/adr/0016-native-ui-and-start-select-shortcut.md` | Dated architecture decision |
| `docs/history/baselines/ALPHA10_BASELINE.md` | Historical evidence |
| `docs/history/baselines/ALPHA11_BASELINE.md` | Historical evidence |
| `docs/history/baselines/ALPHA12_BASELINE.md` | Historical superseded candidate |
| `docs/history/baselines/ALPHA13_BASELINE.md` | Historical superseded candidate |
| `docs/history/baselines/ALPHA14_BASELINE.md` | Historical evidence |
| `docs/history/research/FEATURE_COMPARISON.md` | Current comparison/plan |
| `docs/history/baselines/ALPHA9_BASELINE.md` | Historical evidence |
| `docs/history/baselines/BROWSER_BASELINE.md` | Historical evidence |
| `docs/history/baselines/EFFECTS4_BASELINE.md` | Historical evidence |
| `docs/history/baselines/EFFECTS_BASELINE.md` | Historical evidence |
| `docs/history/baselines/EQ_BASELINE.md` | Historical evidence |
| `docs/history/research/FULL_PORT.md` | Current guidance |
| `docs/history/research/GRAINBERNIK_COMPARISON.md` | Current guidance |
| `docs/development/MIYOO_HARDWARE.md` | Current guidance |
| `docs/history/baselines/MOTION_BASELINE.md` | Historical evidence |
| `docs/history/research/POCKETTRACKER_RESEARCH.md` | Current guidance |
| `docs/development/RELEASE_READINESS.md` | Current guidance |
| `docs/development/RELEASING.md` | Current guidance |
| `docs/development/REMOTE_DEBUGGING.md` | Current guidance |
| `docs/development/REPOSITORY_LAYOUT.md` | Current guidance |
| `docs/development/RESUME.md` | Current guidance |
| `docs/history/baselines/SYNTH_BASELINE.md` | Historical evidence |
| `docs/history/baselines/TAPE_BASELINE.md` | Historical evidence |
| `docs/history/DEVICE_TEST_THROUGH_ALPHA9.md` | Historical evidence |
| `docs/history/GRANULAR_GUIDE_THROUGH_ALPHA9.md` | Historical evidence |
| `docs/history/TARGET_PLATFORM_INITIAL.md` | Historical evidence |
| `docs/history/TESTING_2026-11-17.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.1.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.10.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.11.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.12.md` | Historical superseded candidate |
| `docs/releases/0.1.0-alpha.13.md` | Historical superseded candidate |
| `docs/releases/0.1.0-alpha.14.md` | Historical superseded candidate |
| `docs/releases/0.1.0-alpha.2.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.3.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.4.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.5.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.6.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.7.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.8.md` | Historical evidence |
| `docs/releases/0.1.0-alpha.9.md` | Historical evidence |
| `README.md` | Current guidance |
| `ARCHITECTURE.md` | Current guidance |
| `CONTRIBUTING.md` | Current guidance |
| `LICENSE_SCOPE.md` | Current guidance |
| `CHANGELOG.md` | Current guidance |
| `SECURITY.md` | Current guidance |
| `CODE_OF_CONDUCT.md` | Current guidance |
| `AGENTS.md` | Current guidance |

Alpha15 adds ADR0017, ALPHA15_BASELINE.md and release notes for reset scopes and
held controls. Current user guides describe these; earlier candidate records remain historical.

Alpha16 development adds ADR0018, VIDEO_FEATURE_ROADMAP.md and development release
notes. Transcript provenance/limits are explicit; projected features are not claimed
as implemented. Device acceptance and artifacts remain alpha15 until deployment.

Alpha16 slice implementation adds ADR0019 and ALPHA16_BASELINE.md. User guides
describe generated patterns and shared BPM, clearly separating future manual steps,
collision visuals and phase-locked effects. Current settings11 preserve old1–10.

Alpha17 adds ADR0020, ALPHA17_BASELINE.md and release notes for the studio UI and
wall-collision source. README/current guides now describe X grouped modules, BALLS/
BOWL and settings12; historical baselines retain their old mappings as evidence.
Host-rendered original examples regenerated once; collision image is original, not
an imported reference screenshot. Technical constraints remain outside README.

Alpha18 adds ADR0021, ALPHA18_BASELINE and release notes for actual owner feedback.
Current guides describe contiguous groups/STYLE/palette save/cancel,20Hz deadlines
and separate theme storage. Historical17screens/timing remain labelled as evidence.

Final alpha18 acceptance updates current progress/device/resume guidance and adds
NEXT_INSTRUMENT_REPORT.md. The active progress table replaces a stale alpha14
summary; versioned evidence remains historical. Releasing examples now use18,
current UI documentation includes STYLE and old15FPS descriptions stay in history.
Launcher brand colors match default Burgundy; palette changes do not rewrite it.

Alpha19 adds ADR0022 and release notes for temporary sound recall; README, sound
guide and in-app help describe SELECT-first A/B, source invalidation, transport
preservation and the temporary slot. The next-feature report marks this priority
implemented. Snapshot and current source/device acceptance are recorded separately.

Alpha20 adds ADR0023/baseline/release notes and user guidance for DRONE, its WAV/
granular-only scope and scan hold. Current storage guidance moves to13/114fields,
keeping older format/acceptance reports historical. Next report marks the first
ambient modulation stage implemented; dual/wavetable, manual patterns and export
remain future work. No live freeze/second layer/extra voice claim is made.

Alpha21 corrects the owner's intent: DRONE is now a standalone generative synth;
WAV modulation is EVOLVE. README, controls, notices, report and credits distinguish
them. Settings14/120fields and old-file preservation are explicit. The M5Burner
reference is resolved via the matching MIASMORA author link, with no material reuse.
The next-feature report is rewritten around useful scenes/textures/patterns/export.


## Maintenance audit — 2026-11-19

Consolidated current architecture, progress, tests and user README; retained the
previous documents under history with adjusted links. Corrected schema16/132 counts,
patch compatibility, current drone diagnostic durations, 20 Hz rendering and the
alpha22-device/alpha23-candidate distinction. Recorded successful SSH compatibility
and original WAV transfer. Added SDK offline restoration and generated-output
retention/recovery guidance. Versioned baseline/release/ADR evidence remains dated;
old counts there are historical facts, not current instructions. Documentation links,
source allowlist and all 440 original reference hashes are checked after cleanup.
