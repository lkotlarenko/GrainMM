# ADR 0003 — Descriptor-anchored files and bounded parameter persistence

Date: 2026-11-18. Status: implemented; physical save/reopen acceptance pending.

Onion's existing audio preload exports an openat wrapper that ignores its
parent descriptor. Four controlled ARM dummy-driver cases isolated the failure:
preload alone broke WAV loading; library search order alone did not. Read-only
symbol/disassembly inspection corroborated it. Do not unset the audio preload.

Open the chosen directory with O_DIRECTORY/O_NOFOLLOW, retain its descriptor,
then open `/proc/self/fd/<descriptor>/<basename>` with O_NOFOLLOW. This preserves
the live directory anchor and final-file guards while avoiding the broken wrapper.
Linux procfs is verified on the target; missing procfs fails explicitly. A host
fault fixture emulates wrong-CWD resolution, including a same-name decoy sample.

Store only the six original grain parameters in `state/parameters.v1`: a strict
`GrainMM 1` header followed by six finite, in-range floats in the classic locale,
with lossless float precision. Maximum input is512bytes; reject trailing data,
unknown schemas, nonregular files and symlinks. This is a small GrainMM preset,
not a claim of GrainBrick project compatibility. Sample/audio ownership is unchanged.

Read before audio starts. Save edited values on normal exit after audio pauses;
never perform storage work in the callback. Signal/power exits skip saving.
Write exclusively to `parameters.pending`, flush/fsync, then rename atomically
within the same anchored directory. Preserve failed pending transactions and the
old valid preset; refuse another write until the owner moves the pending file
aside. A completed rename followed by directory-flush failure is reported as an
uncertain durability result, not a guarantee the old version remains. Filesystem/
power-loss guarantees depend on the SD/FAT implementation; this is not a journal.
No automatic migrations, periodic writes or sample modifications.

Host tests cover numeric roundtrip, normal replacement, unfinished save, corrupt/
unsupported/extra data, invalid bounds, symlink protection and live save/reopen.
The live suite also runs under the original preload fault fixture; the normal
suite runs ASan/UBSan/LSan. Hardware testing and power behavior are separate.
