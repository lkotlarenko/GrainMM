> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Sample browser and interface acceptance — 2026-11-18

Version0.1.0-alpha.3, immutable target release `f8ebc22ce5fb`.
ARM appSHA256 `f6cdc3f767cd15a0f3051fbf8097849ba154cbe5f32080379e423d86b62b8a29`.
Install ZIP64016bytes, SHA256 `59691c1bbab70ef85b9aa07b5b2c3e4ea318c91eda4557f0ebb9bd4caf82c032`.

## Implemented and verified

- Rooted read-only WAV/folder catalog, Y navigation, A load, B back, X tone.
- Control-thread decode and transactional renderer replacement; no allocation,
  files or app locks in the audio callback. SDL1.2 mixing lock protects ownership.
- v3 source/control persistence, preserved v1/v2 migration, strict corrupt-state
  refusal and missing-source recovery without overwriting saved selection.
- Six visible page controls with selection highlight; metrics stay in logs.
- Original host-rendered README screenshots with illustrative filenames, explicitly
  labeled as host captures. One new layout image was inspected; no new device
  screenshots or repeated navigation photographs were used.

Host13CTest suites and sanitizer12suites passed (ASan/UBSan/LSan outside ptrace).
Tooling8/hardware-tools8/runtime4 passed. ARM build/ELF/ABI checks passed against
SDKv0.0.3/GCC8.3; new fcntl needs GLIBC2.28, verified on target. Repeated ZIP builds
were byte-identical. Reference check preserves all440baseline inputs.

Four bounded native SDL-dummy cases under stock libpadsp/search paths passed:
explicit WAV→browser load; saved-file resume; tone→browser WAV load; and missing
stored WAV recovery. Selection persisted, v2 and original WAV hashes stayed
unchanged. Case callback maxima were2251,2971,2063,1130us; late hints0 and grain
drops0. These dummy runs are neither audible tests nor hardware latency measures.
Temporary fixtures were removed by exact name after hash checks.

## Physical owner acceptance

The owner confirmed upright/readable six-control display, Y browsing, selecting
sample.wav, loading/playing, X page and D-pad edits, MENU exit and reopen; audio
stayed clean. One completed foreground log records3200callbacks,3276800frames,
1024frames/callback, span74369ms, worst6828us, late-hint0,341grains,0grain drops,
164updates and2source changes. UI1484requests/195presentations. Backend XRUNs remain
unavailable; do not infer zero underruns from scheduling counters.

VNC remained at owner-selected2FPS and SSH was enabled. No VNC-off CPU baseline,
new RSS sample, end-to-end latency or headphone/suspend/power acceptance was
collected this increment. Five-minute clean granular listening belongs to the
previous alpha1 build; it is not a five-minute effects/browser soak.

## Storage and rollback

CURRENT/LAST_GOOD selectf8ebc22ce5fb; verifiedede6c953a072 remains the only rollback.
Older029c82a5a13c matched its host archive before exact-file nonrecursive pruning.
The app-only launcher was hash-checked, backed up in bootstrap-before-browser and
atomically replaced; its argv remains compatible with the rollback binary.
No Onion files, mixer, CPU policy, services or other applications were changed.
App footprint2336KiB after acceptance; SD free about4.7GiB before deployment.
Owner v1/v2 hashes are unchanged; v3 contains the selected sample and edited values.
All raw device data/endpoints/approval records stay private on the host.

## Remaining release gates

Hosted CI has not run; source push/tag/publication require explicit authorization.
Longer combined-effects listening, headphone/speaker coverage, VNC-off resource
measurement and owner-assisted sleep/power checks remain stable-release work.
Broader DSP/project coverage follows FULL_PORT.md; this is not a full GrainBrick
feature-parity release.
