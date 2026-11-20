# Alpha23 — output headroom and useful ambient starting points

> Historical evidence. Commands, feature gaps and device selectors describe this report's date. Use [current status](../../PORTING_PROGRESS.md) for today's build.


2026-11-19. Owner confirms alpha22 sound/UI improvement and requests FX volume.
New OUTPUT gives global post-rack gain and linked sample-peak limiting, retaining
older gains by default. Eight original drone scenes speed sound creation without
implicit source/FX/level/transport changes. ADR0026 records contracts and limitations.

## Host level audit

48kHz stereo, fixed original drone LEVEL0.6/root60/major/texture0.6/movement0.6,
event mix0.35/rate0.7. First47×1024frames discarded,235blocks measured. These are
PCM RMS/peak, not acoustic SPL or universal effect compensation. No clips occurred.

| Test | RMS dBFS | Peak dBFS |
| --- | ---: | ---: |
| Dry |−22.45|−13.06|
| EQ all bands+6dB |−14.22|−4.74|
| Compressor MIX0.5 |−22.55|−13.24|
| Tape MIX0.5/drive6/output0 |−21.92|−12.99|
| Crush MIX0.5 |−22.47|−13.13|
| Filter MIX0.5/cutoff2000 |−22.54|−13.22|
| Chorus MIX0.5 |−25.51|−14.15|
| Resonator MIX0.5 |−28.45|−19.02|
| Tremolo0.6 |−25.19|−13.14|
| Delay MIX0.5 |−26.17|−16.22|
| Reverb MIX0.5 |−22.25|−11.54|
| Reverb0.5/delay0.3/output+6/limiter |−20.33|−9.46|

The output stage controls combined level without a permanent boost in each effect.
Numerical tests check+6dB≈1.995×amplitude independently, neutral bit identity,
linked limiting, mono, recovery, prepared bounds, block invariance and allocation.
Initial tests caught mono interpolation residue and stale schema expectations;
fixed without disabling checks. Expanded source/modal/play/save probe passes.
Final host/sanitizer/ARM/native/artifact evidence is maintained in RESUME.md.
All-FX scheduling outliers, long listening/headphones/power and VNC-off resources
remain release gates. No screenshots, device-wide modifications or public release.

## Final host checkpoint

The exact-code279-file source draft builds and passes all32host tests in161.36s.
Full31ASan/UBSan/LSan cases pass127.97s outside ptrace, leak checks retained.
Runtime5/tooling8/hardware8/source2, SDK ARM/ELF/deps, format/analyzer/source/docs and
all440reference hashes pass. Final install208290bytes; repeat byte-identical.
SOURCE/OUTPUT defaults and prior state contracts remain neutral unless edited.

Candidate40f94504ec9d is staged on the host only. SSH dropped during interactive
login, so deployment/native measurements and owner listening are pending; current
alpha22plus verified18rollback remain unchanged. Prepared26app/2numerical probes
must run sequentially after the connection is restored. No screenshots or new SD
builds were added this session. No stable/native publication is implied.
