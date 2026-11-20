# Repository layout

The user requested the root cleanup. All440 baseline inputs were verified after
reversible relocation; no original content or Git history was discarded.

- src/{app,audio,dsp,model,platform}: original native implementation by responsibility.
- include/grainmm: public interfaces; tests and benchmarks remain separate.
- packaging/onion: app launcher/config; state stays inside the approved app root.
- scripts/build.sh: build entry point.
- scripts/{toolchain,packaging,maintenance,miyoo}: SDK/ELF, packaging, preservation gates and device tooling.
- docs: indexed current guidance; docs/development contains current handoff/release/hardware/debugging notes,
  docs/adr dated decisions, docs/releases version notes and docs/history/{baselines,research} dated evidence and prior guides.
- .local/reference/grainbrick-beta2: ignored original payload, including original README.
- .local/guides: private master prompt and feasibility research, never published.
- .local/baseline: untouched recovery archive/checksums plus private relocation map.
- .local/hardware: private connections, approvals, raw evidence, plans and navigation journal.
- build/{host-debug,host-asan-ubsan,miyoo-armv7-release}: retained active outputs.
- build/logs/v1: current candidate build/test evidence; alpha23 logs remain historical.
- build/archive/{source-rebuilds,evidence,packages,experiments}: recoverable historical generated material.
- dist: frozen canonical release artifacts; never silently overwrite them.
- .local/sdk: pinned compressed SDK; restore extraction offline before ARM builds.
- .local/maintenance: private audit inventories, checksums and relocation maps.
- .local/audio: original generated WAVs, generator and transfer verification.

The new root README is for beginners: purpose, current status, install/run,
controls and credits. Advanced commands and audits live in linked documents.
GitHub-standard contribution/security/conduct and license files remain at root.
The original upstream README is retained in reference storage and initial Git
history. verify_reference.py checks its original hash rather than requiring it
as a suffix of the new user-facing README.

See [workspace maintenance](WORKSPACE_MAINTENANCE.md) for disk usage and safe recovery.
