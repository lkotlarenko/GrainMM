# ADR0012 — GPLv3 and matching source

Date:2026-11-18. Owner explicitly authorized the license switch.

Current reviewed GrainMM work uses **GPL-3.0-only** from alpha10. Future GPL
versions are not automatically selected. Original GrainBrick/third-party terms
and copyright notices are unchanged. Earlier MIT grants stay valid and the
original MIT notice remains in source/binary packages; no history is rewritten.
License documents retain their own copying terms rather than being relicensed.

The verbatim GPLv3 text is from SPDX's official license-list-data repository,
text/GPL-3.0-only.txt. LICENSE_SCOPE.md defines current scope. No upstream binary,
Python, sample, font or preset was imported to perform this switch.

A public binary must be paired with matching Corresponding Source/build materials.
The exact allowlist source archive excludes private guides, Git history, SDK,
credentials, endpoints, raw captures/logs and reference payload. It records file
hashes, uses deterministic timestamps/modes and refuses overwrite or symlink paths.
CI prepares installation + source + checksums; publication checks a separate owner-
authorization flag. A GPL choice does not authorize publishing private alpha code.

Current public policy remains documentation only. Native source/binaries need new
explicit owner authorization before branch/tag/release publication. Private local
archives let the owner inspect/test corresponding code without public upload.
