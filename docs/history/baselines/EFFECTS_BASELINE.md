> Historical evidence. Current checkpoint: [RESUME.md](../../development/RESUME.md).

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


# Grain-plus-delay baseline — 2026-11-18

Native Miyoo Mini Plus / OnionOS4.3.1-1, SDKv0.0.3 GCC8.3 -O2,
cortex-a7/vfpv3-d16/hard; VNC2FPS and SSH enabled. No CPU policy/settings changes.
Offline synthetic stereo48kHz,512-frame blocks,281blocks/case (3simulated seconds).
Both default12grains/s80ms and dense80grains/s250ms workloads include the actual
portable Instrument graph, final PCM conversion and output fade. Delay:450.5ms,
feedback0.95,ping-pong1,spread1,LP800Hz,mix0or0.65. Ring storage768016bytes.

## Targeted grain-window optimization

Precompute the Hann lookup phase multiplier when a grain starts, replacing a
per-active-voice/per-sample double division with multiplication. Explicit final
window endpoint remains zero. No change to source interpolation, pitch, envelope
LUT, random sequence, voice policy or gain normalization. Allocation-free tests,
analytical Hann reference at4rates/3sizes, arbitrary-block determinism and a
2second stereo render comparison pass; the PCM comparison is bit-identical.

Old/new/old native comparison, observed sysfs clock was
**400000kHz =400MHz** before/after every run (not a locked-clock experiment):

| Dense stereo case | Old p95 µs, pass1 | New p95 µs | Old p95 µs, pass2 |
| --- | ---: | ---: | ---: |
| Dry delay mix0 | 6173.500 | 5264.500 | 6105.500 |
| Wet mix0.65 | 5977.667 | 5261.333 | 5987.166 |

Wet-case improvement is about12%; sampled checksums are identical in all4cases.
The subsequent installed optimized candidate measured wet dense p95=5270.334us,
p99=5307.500us,max=5318.667us. Default wet p95=1143.333us,max=1205.333us.
The arithmetic512-frame deadline is10666.667us. Outliers in old trials exceeded
that deadline (e.g.12731.333us): these are wall-clock process measurements under
background services, not a worst-case bound or hardware underrun count. More
cases, foreground measurements, longer soaks and VNC-off testing remain needed.

## Physical interface and audio observations

Owner confirmed X switches GRAIN/DELAY, MIX changes the sound, saved values survive
exit/reopen, and no audio problems were heard in this short check. Actual saved
v2 MIX was0.300000012. The original v1 file remains unchanged and usable by the
older verified build; native migration fixtures verified this by SHA256 and were
removed after passing. Only current and one verified rollback remain on SD.

Audible foreground session:3556callbacks,226grains,0voice drops,7updates,
late-hint0,max8432us; produced3641344frames, observed callback lengths1024/1024,
monotonic span75840ms. This is consistent with roughly48kframes/s produced by the
callback; it does not directly measure DAC clock or end-to-end latency. A later
silent editing/reopen session had1066callbacks,12updates,max320us and normal exit.
The SDL loop averaged about20present requests/s despite a25fps ceiling; do not
infer elapsed time by multiplying UI request count by the minimum40ms delay.
Actual backend XRUNs remain unavailable. Owner listening and backend counters
are reported separately.
