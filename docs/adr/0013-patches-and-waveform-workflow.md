# ADR0013 — Named patches and cached waveform

Date:2026-11-18. Implemented; physical acceptance recorded separately.

## Patch format and storage

Use16fixed slots under state/patches, with a20character uppercase name and the
strict66control/source preset. Patch format1 wraps settings format8; ordinary
parameters.v8 remains unchanged for alpha9 compatibility. WAV data is referenced
by rooted relative path, not duplicated. This is native GrainMM storage, not
GrainBrick project compatibility.

START pauses audio and opens patches. Up/down chooses; A loads, X saves, Y edits
the name for the next save. Name edits are drafts until X; occupied slots need a
second separate X press. Held-button repeats cannot authorize replacement.
B cancels confirmation/name editing before closing; MENU saves ordinary settings
and exits without committing an unfinished patch draft. No delete action.

Codec validation is independent from filesystem code and reused by autosave and
patches. PatchStore retains a directory descriptor and uses procfs-anchored paths
because Onion's inherited padsp loses openat's dirfd. It enforces fixed filenames,
regular/no-symlink bounded files, strict names, exclusive pending writes and fsync/
rename. Failed transactions preserve old committed data and unfinished pending
files for recovery. Never silently promote or remove them.

Patch load decodes the new WAV and builds its preview/engine before swapping
ownership. Any failure leaves the previous sample/parameters intact and paused.
Successful loads start silent. Old samples outlive the callback's old graph.

## Waveform and performance

B with a loaded WAV opens SAMPLE waveform trim. Up/down selects a sample control,
left/right edits, A plays/pauses, B returns; page switches leave the view. The view
uses256min/max bins across both input channels. Build once when a source changes;
rendering reads the fixed2KiB cache. Region edits never rescan or modify audio.

No additional DSP, source copies or callback I/O/allocation/locks are introduced.
WAV decoding already uses fixed4KiB reads and skips unrelated chunks, so no decoder
rewrite is justified. Keep measured loading and callback/resource evidence separate.

## Modules and verification

main only dispatches; options parses without SDL; application owns lifecycle/view
orchestration. PatchMenu and Waveform are portable models, storage is separate from
the codec, extra views are separate from basic drawing. Dedicated tests cover
atomicity, interrupted/corrupt saves, directory rename anchors, replacement/name
states, min/max preservation, cached views and full save/load/trim UI navigation.
