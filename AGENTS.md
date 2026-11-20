# GrainMM development contract

Build a compact Miyoo Mini Plus / OnionOS sound instrument. GrainBrick is by
ETRNLIAM: https://github.com/ETRNLIAM/GrainBrick. GrainMM is an experimental port
project, not the original implementation or its release history.

Read docs/CODE_QUALITY.md, docs/PORTING_PROGRESS.md, docs/AUDIT.md, docs/LEGAL_AND_PROVENANCE.md,
docs/TARGET_PLATFORM.md and docs/adr/0001-platform-and-runtime.md first.
Private master/research guides may exist locally; never stage or publish them.
Do not delegate to sub-agents unless the user explicitly requests it.

## Preservation and publication

Never git clean, destructive reset, delete or overwrite the reference payload, bulk-stage
files, rewrite history, push, tag or publish without explicit authorization.
.local/baseline contains private initial hashes, Git state and recovery archive.
Original files were moved with user authorization into .local/reference/grainbrick-beta2.
Private guides are in .local/guides; relocations.json maps all baseline paths.
The immutable baseline snapshot and reference hashes must remain intact. Original README bytes are preserved in the ignored reference payload and initial Git history.
Never add original Python, binaries, samples, presets, fonts or images to src or
packages. GPL-3.0-only applies to the reviewed current GrainMM work, excluding
license texts and upstream evidence. Prior MIT grants remain valid; preserve
LICENSES/GrainMM-MIT.txt and third-party notices.
The user confirmed full port/modification/distribution permission on2026-11-18,
with attribution to ETRNLIAM. Do not repeat the obsolete permission blocker or
request private correspondence. Preserve separate third-party licenses; the grant
does not blanket-relicense upstream materials as GPL. Technical release gates remain.
Never mention or solicit non-public communications; use the cited public discussion.

## Product direction and iteration pace

The owner clarified that musical quality and useful feature coverage matter more
than a bit-identical or line-by-line port. Use independently authored practical
native equivalents; compare upstream behavior only when it resolves a real issue.
Prefer larger working feature batches with a short build/smoke-test gate and a
verified rollback. Do not delay useful deployment for exhaustive matching; extended
soaks follow working builds. Real-time safety, state preservation and device
boundaries still apply. Never claim unavailable or untested features work.

## Current publication scope

On2026-11-20 the owner explicitly authorized final review and publication of the
reviewed native source, build scripts, CI and1.0.0-alpha.1 prerelease, with images
and a roadmap. This supersedes the earlier documentation-only scope. Publish only
reviewed allowlisted files from the existing public main history; preserve private
native development history on grainmm/native-device-proof. Never include private
reference/guides, credentials, captures, SDK or owner data. Future releases and
changes beyond the authorized release still require applicable authorization.
Keep stable_ready false until actual stable acceptance; label the alpha honestly.

## Architecture and execution

C++17 portable DSP; host SDL2 and provisional SDK SDL1.2 target adapters.
No GPU dependencies, Python runtime, native GrainBrick libraries or copied DSP.
Audio callback: no I/O/logging/allocations/locks/drawing/parsing; only bounded
lock-free controls and preallocated work. Smooth transitions. Start silent.
MENU/Escape exits; A/Space toggles playback. With no WAV or enabled oscillator/collision/drone source, the fallback is a gentle 220Hz tone. No CPU/volume/rootfs changes.
Maintain RAII close-before-state-destruction. Hardware evidence outranks assumptions.

Build: `sh scripts/build.sh host-debug`; `sh scripts/build.sh host-asan-ubsan`.
Sanitizers need an environment without ptrace; do not silently disable leak checks.
SDK: `python3 scripts/toolchain/fetch_sdk.py` (Python >=3.12, gh); then
`GRAINMM_SDK="$PWD/.local/sdk/miyoomini-toolchain" sh scripts/build.sh miyoo-armv7-release`.
Format new C++ only with clang-format; never bulk-format reference code.
Granular sample/queue contracts: ADR0002 and docs/GRANULAR_PROTOTYPE.md.
Tests: `ctest --preset host-debug`, `ctest --preset host-asan-ubsan`,
`python3 tests/tooling_test.py`. Gate: `python3 scripts/maintenance/check_source.py`.
Local-only package: `python3 scripts/packaging/package.py --local-experiment` (never overwrites).
Before commits: explicit paths, source gate, diff/check, private-reference hash check.

## Continuity and definition of done

Record each session's commands/evidence/blockers and exact next action in progress;
record architectural changes in ADRs. Distinguish static/host/ARM/QEMU/physical tests.
No physical success claims from cross-compilation. Callback lateness is not an XRUN.
P2 starts after P1 physical investigation or as separately tested portable work;
no fake controls. Acceptance needs actual MM+ stable granular audio, save/reopen,
controls/exit/power handling, measured CPU/RSS/underruns and distribution clearance.

## Physical-device boundary (latest user specification)

Read-only SSH discovery and existing VNC viewing/harmless navigation are authorized.
The first deployment requires explicit user approval; prepare/test/dry-run first.
Default development root is /mnt/SDCARD/App/GrainMM, including state/logs/samples.
No writes to Saves or any directory outside that approved root. Keep only current
and verified rollback builds on SD after validation; archive diagnostic builds on
host and hash-check exact app-owned files before pruning, never broad deletion. Record approval
privately before --apply, with scope and whether ongoing iterations were granted.
Never modify Onion runtime internals, services, SSH/network settings, audio mixer,
firmware or other apps; never stop critical processes or reboot without specific
authorization. Never put credentials/endpoints/captures in tracked files.
Original payload was reorganized at the user's explicit request with all 440 hashes
preserved and a private relocation map. Source scripts are grouped under
scripts/{maintenance,toolchain,packaging,miyoo}; scripts/build.sh is the build entry.

## User-facing documentation and screenshot policy

README is a beginner-friendly user guide: purpose, honest current status, simple
Miyoo installation, first sound, controls and credits. Keep compiler commands,
benchmarks, legal inventories, debugging and architecture in linked documents.
User explicitly requested very few image inspections. Prefer logs, process state,
input-only VNC and the private navigation/event journal. Capture/analyze images
only for essential first-time UI validation or a rendering bug; reuse evidence.
Do not repeatedly photograph navigation. Record successful key sequences and
known UI state privately so future sessions need fewer screenshots.

## Pause and next-session handoff

Development resumed at the user's request on2026-11-18; the previous pause is revoked. Start with
docs/development/RESUME.md; it records the known-good device selector, preserved
fixture, completed loader/preload fix and verified effects checkpoint.
Remote scripts stream over SSH stdin; large exec requests reset this embedded
server in observed probes. Never change SSH services/authentication for automation.
Credit the port as elly (@lkotlarenko), alongside ETRNLIAM's original GrainBrick credit.

## Workspace maintenance

Historical rebuilds and loose generated evidence are archived under build/archive;
latest prior alpha23 logs are under build/logs/alpha23. Private relocation and hash
manifests are in .local/maintenance. The extracted SDK is compacted after validation;
restore the retained pinned archive with scripts/toolchain/fetch_sdk.py before ARM
compilation. Current maintenance checks are recorded in RESUME.md. Preserve frozen
release archives at their original source commits; rebuild/repackage changed code.
