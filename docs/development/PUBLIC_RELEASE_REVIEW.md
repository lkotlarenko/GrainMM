# Public alpha release review

Date: 2026-11-20. Version: **1.0.0-alpha.1**. This is the first public native
GrainMM prerelease, not a stable release. The owner explicitly authorized final
review and native source/CI/release publication, with images and a roadmap.
This supersedes the earlier documentation-only scope.

## Scope and preservation

Publication starts from the existing public main history and copies only reviewed
allowlisted source, tests, scripts, documentation, original screenshots and license
notices. Private development history, SDK, reference payload, guides, connection
settings, owner WAV/state/exports and raw device captures are excluded. Original
440-file reference hashes remain verified in the original workspace. Frozen earlier
local archives retain their original bytes/source provenance.

## Review coverage

Every allowlisted file is inventoried for path/type, content hash, license/provenance,
secret/private-input patterns and build/archive inclusion. Runtime review focuses
on callback ownership and bounded queues, immutable samples/source replacement,
parameter preparation, strict versioned state, rooted loading, exclusive WAV export,
output limiting and new synthesis/groove. Clang Static Analyzer covers all47runtime
translation units. Host/sanitizer/integration and adversarial archive tests provide
execution coverage; this is not a claim of a formal proof or security certification.
Dated research, ADRs and development notes retain historical context.

## Final-review corrections

- Replace obsolete native-publication blockers with the owner's current scope.
- Correct the user guide's BPM range to30–240, matching actual controls.
- Replace misleading TIMBRE dry/bypass artwork with table/sub state; show actual
  OSC B settings and a GROOVE phrase overview instead of generic source artwork.
- Update source labels for oscillator B/sub-only operation.
- Add current host-rendered TIMBRE/GROOVE/DRONE screenshots and a visible
  “What's next” section; keep screenshots distinct from device evidence.
- Add exact matching-source release verification against the tagged checkout,
  plus tests for corruption, source substitution, unsafe paths, versions and links.
- Remap installed guide/license links to their packaged filenames or the exact
  public tag; USER_GUIDE.md and the flattened license files retain working links.
- Render release-note links and images against the exact tag so GitHub Releases
  displays them correctly. Keep GPL source notice consistent with public metadata.
- Resolve GCC13's sanitizer-build maybe-uninitialized diagnostic for a conditional
  array selection in the lifecycle probe by indexing the two arrays in explicit
  mode branches. Keep warnings-as-errors and all sanitizers enabled.
- Lint prepared workflows and verify pinned Actions/SDK references. Enable bug
  reporting in the public repository to match documented support links.

## Verification and publication procedure

The previous [feature validation](V1_VALIDATION.md) passed36host/35sanitizer tests,
ARM/static/tooling and independent matching-source builds. Final-review changes
are rechecked locally and by hosted CI on the publication commit before tagging.
Release automation repeats CI and verifies installation + matching source + both
checksums before creating an experimental GitHub prerelease. Downloaded public
assets must pass the same verifier. The workflow/run URLs are available through
[Actions](https://github.com/lkotlarenko/GrainMM/actions); exact local review logs
and artifact manifests remain ignored.

## Remaining device gates

The device was unreachable during preparation. Alpha25 remains the accepted
physical checkpoint; new1.0DSP/groove and the final UI correction require their
own device listening/navigation checks. Longer mixed-source/headphone/speaker,
power/sleep, error recovery and VNC-off CPU/RSS checks remain open. No hardware
underrun count is exposed. Stable_ready stays false; release notes disclose these
limits. [What's next](ROADMAP.md) prioritizes that work before further features.
