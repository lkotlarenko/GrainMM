# Workspace maintenance

Previous maintenance audit: 2026-11-19. No new features, target-device files, Git history, reference inputs or original
test WAVs changed. The code pass adds an exceptional-input OUTPUT guard and removes
a redundant diagnostic initialization; normal signal behavior remains covered by tests.

## Disk usage and retention

The initial checkout occupied about 2.8 GiB. Most space was an extracted cross SDK
and repeated independent-source build trees, rather than the source code itself.
After that cleanup it occupied about 966 MiB; exact local measurements and recovery
manifests are retained in ignored `.local/maintenance`.

| Material | Decision |
| --- | --- |
| Original reference, private guides, baseline/recovery archive | Retain untouched; all 440 baseline inputs verified |
| Git history, current source and tests | Retain; no history rewrite or runtime changes |
| Pinned SDK archive | Retain, SHA-256 verified |
| Extracted SDK | Remove only after every archived file/link matched and no extras were found; 5,984 regular files checked |
| 17 historical source rebuild trees | Compress, verify every member/content, then remove only the exact archived entries |
| Current host, sanitizer and ARM output directories | Retain |
| Canonical `dist` releases/source archives | Retain unchanged, tied to their original commits |
| 471 loose generated files | Relocate without content changes, with a private SHA-256 map |
| Device logs/approvals, research tools, screenshots, original WAVs | Retain; no image inspection or speculative deletion |

The SDK archive is approximately 267 MiB; historical rebuild archives occupy about
211 MiB. Keeping these recovery materials is intentional. They are not published.
No automatic age-based deletion or broad recursive cleanup was introduced.

## Generated layout

- `build/host-debug`, `build/host-asan-ubsan`, `build/miyoo-armv7-release`: current outputs.
- `build/logs/alpha23`: preserved alpha23 build/test/analyzer logs; current logs are
  `build/logs/v1`.
- `build/archive/source-rebuilds`: verified historical rebuild snapshots.
- `build/archive/evidence`: older logs, reports and analyzer outputs.
- `build/archive/packages`: package experiments and repeated artifacts.
- `build/archive/experiments`: preserved standalone experiment binaries/scripts.
- `dist`: canonical installation and source artifacts.

Historical instructions may name the original flat `build` paths. Consult
`.local/maintenance/output-relocations.json` for their exact new locations. Rebuild
archive inventories, per-file hashes and original paths are in `archive-index.json`.
Do not execute an old maintenance script without reviewing its stored paths.

## Restore the SDK offline

```sh
python3 scripts/toolchain/fetch_sdk.py
```

With the cached archive present, this verifies and extracts locally without a
network download. Budget about 1.5 GiB of additional disk space. The archive SHA-256
is pinned by the script. Then use the normal [ARM build](../BUILDING.md) command.
Do not strip or manually prune compiler/sysroot contents to save space.

## Recover old experiments

Each historical archive contains its original directory name. Inspect its members
first, then extract into a new empty recovery directory, never over existing work.
For an archive-only restoration, use Python's `tarfile` extraction with the `data`
filter and compare against the private manifest. Restore to `build` only after
checking that the old destination is absent. Current source rebuilds can instead be
regenerated from the corresponding retained source archive in `dist`.

## Maintenance validation

Run documentation/source/reference gates, inspect `git diff --check`, and run
focused host checks when moving generated build material. Documentation-only edits
do not justify speculative DSP optimizations or a device deployment. Keep dated
baselines and ADRs intact; current guidance belongs in the documentation index,
architecture, progress and resume documents.

## Code maintenance follow-up

The owner explicitly included code cleanup and bug fixes. All 42 implementation
translation units were checked with Clang static analysis; a redundant initialization
in the lifecycle diagnostic was removed, leaving no analyzer findings. Every source
translation unit is referenced by CMake; no orphan implementation files were found.
Existing app/audio/DSP/model/platform boundaries were retained rather than rewritten.

A new failing-before regression reproduced OUTPUT overflow on extreme finite inputs:
512 non-finite results in the probe despite limiting. Exceptional inputs are now
bounded before matrix/gain math; the probe produces zero non-finite results. No
ordinary sound controls or DSP algorithms were changed. An independent before/after
comparison of 153,600 stereo frames across six output settings is bit-identical for
normal-range inputs. This is a correctness fix, not a claimed CPU speedup.

All 32 host and 31 sanitizer cases pass; ARM build/ELF checks pass. Tooling, runtime,
source/archive/docs and original-reference gates pass. Exact evidence is in RESUME.md. The SDK was temporarily
restored offline for ARM validation and compacted again afterward. Frozen alpha23
artifacts predate this code fix; use a fresh output filename when repackaging.
No app was deployed or performance benchmark invented during maintenance.


## Major milestone preparation — 2026-11-20

Moved dated baseline/research reports into docs/history/{baselines,research}, fixing
relative links and exact source allowlist paths. Removed the superseded SSH guide
redirect after rerouting links and exact generated Python bytecode under scripts/
tests. Reference, baseline archive, frozen releases and owner data remain intact.
The SDK extraction is restored for current ARM compilation, so the older966MiB
measurement is historical. Active evidence is build/logs/v1; generated-file removal
and move inventory remain private in .local/v1. No broad deletion or history rewrite.
