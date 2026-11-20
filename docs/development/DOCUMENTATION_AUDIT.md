# Documentation audit — 1.0 milestone

Updated2026-11-20. Current guidance is separated from dated evidence. The earlier
[audit history](../history/DOCUMENTATION_AUDIT_THROUGH_ALPHA25.md) remains preserved.

## Corrections

- Rewrote README for first-time players: purpose, availability, all source/workflow
  features, installation, sound recipes, controls, backup, limits and credit.
- Replaced contradictory sound/install/recovery instructions: generated-only
  patterns, X-cycles-pages, settingsv10/v12/v16 and outdated release commands.
  Current schema18 has150scalars plus the independent pattern payload.
- Documented major alpha status without implying stability or a public download.
  Alpha25 remains accepted hardware; native publication was authorized on2026-11-20.
- Described stock Bluetooth absence and unsupported MIDI/USB/live input/network
  music features; note numbers do not imply MIDI support.
- Updated architecture, current status/device acceptance, release readiness/process,
  roadmap, test coverage, notices and repository layout together.
- Moved dated baselines/research into history/baselines and history/research;
  repaired relative links and source allowlist entries. Historical claims retain
  their dates and cannot override current status.
- Removed the superseded SSH guide redirect after rerouting all links. Removed
  exact stale bytecode only; source/reference/owner data remain preserved.
- Prepared a standalone documentation-only public review tree and manifest in
  ignored local storage. Public variants do not overwrite native/source guidance.

## Coverage and gates

All repository Markdown (root/docs), packaged notices, license scope, release
metadata, source allowlist and prepared workflow commands were inspected. Unchanged
historical measurements and ADRs remain dated evidence; license text is unchanged.
Links/headings are checked by check_docs.py. Source paths are reviewed explicitly;
440original hashes, diff whitespace and packaging allowlists remain mandatory.
Actual run results are in [1.0 validation](V1_VALIDATION.md).

## Remaining scope

The owner authorized native source/CI and an experimental release on2026-11-20.
The final publication pass verifies hosted CI before tagging; check the workflow
status for actual results. New candidate listening/navigation and extended
stress/power/resource checks remain pending while the device is unreachable.
Private connection details, raw captures and baseline recovery material stay ignored.
