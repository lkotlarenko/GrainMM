# Releasing GrainMM

The owner authorized reviewed native source, CI and the **1.0.0-alpha.1**
experimental prerelease on2026-11-20. Metadata records
`source_publication_authorized:true`; `stable_ready:false` remains until physical
stable acceptance is supported. Future release scope is not automatically granted.

## Local rehearsal

Run from the repository root. Use fresh output names when repeating: packages,
notes and source archives refuse overwrite. Do not create a tag as a test.

```sh
sh scripts/build.sh host-debug
sh scripts/build.sh host-asan-ubsan
python3 tests/runtime_test.py
python3 tests/tooling_test.py
python3 tests/hardware_tools_test.py
python3 tests/source_archive_test.py
python3 tests/release_test.py
python3 scripts/maintenance/check_source.py
python3 scripts/maintenance/check_docs.py
python3 scripts/toolchain/fetch_sdk.py
GRAINMM_SDK="$PWD/.local/sdk/miyoomini-toolchain" sh scripts/build.sh miyoo-armv7-release
python3 scripts/packaging/package.py --release-version 1.0.0-alpha.1
python3 scripts/packaging/source_archive.py --version 1.0.0-alpha.1
python3 scripts/packaging/release_notes.py --version 1.0.0-alpha.1 --output build/RELEASE_NOTES-v1.md
```

For an app-only device experiment use `package.py --local-experiment --output`
with a fresh path. Never deploy an old binary alongside changed source notes.
Record source file hashes, binary/archive SHA256, compiler/SDK and actual checks.
Rebuild/package after source changes; preserve frozen older releases unchanged.

## GPL matching source and public review

Review every path in `packaging/source-files.txt`. Archives exclude SDK, reference
payload, samples, guides, raw logs/captures and connection/approval files. New work
uses GPL-3.0-only; preserve earlier MIT grants, ETRNLIAM credit and third-party
terms. A public binary must ship matching source/build materials. Dynamic device
libraries are not bundled. Fixed archive ordering/timestamps produce deterministic
packaging for identical inputs; cross-host compilation is not claimed hermetic.

Workflows use pinned Actions/SDK. Verify hosted CI on the publication commit
before tagging. Publication uses the existing public main history and a reviewed
allowlist snapshot; private development history, references and device data stay
local. Original-workspace maintainers also run verify_reference.py against the
private440-file baseline before commits; ordinary clones do not contain it.

## Publish an authorized release

1. Review the exact allowlisted source and [release scope](RELEASE_READINESS.md).
2. Run local checks and prepare installation/source/checksums/notes for review.
3. Commit the reviewed snapshot onto public main and wait for hosted CI to pass.
4. Tag only that verified commit. A **v* tag triggers release automation**, which
   repeats CI, verifies authorization/version/notes and compares source archive
   contents against the tagged checkout before publishing all four assets together.
5. Check the published release and download/verify its install/source/checksums.

Use `python3 scripts/packaging/verify_release.py --version 1.0.0-alpha.1` to verify
the complete asset set against the checkout. It rejects wrong versions, asset
corruption, source substitutions, unapproved members and unsafe file types.
Never overwrite an existing release or use a tag as a test. Matching archives are
reproducible for identical inputs; compiler builds are not claimed hermetic.

[Publication review](PUBLIC_RELEASE_REVIEW.md) owns this release's evidence;
[handoff](RESUME.md) owns later development/device actions.
