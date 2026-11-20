# GrainMM roadmap

Updated 2026-11-20. Musical usefulness, a compact native implementation and clear
controls take priority over bit-identical GrainBrick behavior. No feature-parity
percentage is inferred from control counts.

## 1.0 milestone

Candidate **1.0.0-alpha.1** adds independently tuned oscillator B, generated
wavetable morph/scan on oscillator A, sub bass, sample-accurate slice swing,
1–16-step phrases, rotation/reverse/bounce and repeating accents. All use the
existing settings/patch/snapshot/export workflow. SOURCE uses explicit card banks.
These are implemented candidate features; physical acceptance is separate.

## Next: release the useful instrument

1. Validate the exact candidate on MM+: new-source/groove checks, old-patch loads,
   save/reopen, export/resample, controls and exit. Retain verified rollback.
2. Profile new synthesis at realistic and extreme settings; measure CPU/RSS with
   VNC off. Investigate scheduling spikes without calling hints XRUN counts.
3. Run longer mixed-source/headphone/speaker sessions and power/sleep recovery.
4. Rehearse installation and GPL matching-source packaging. Review public scope;
   publish the owner-authorized alpha only after CI verifies exact matching assets.

## After evidence supports it

Consider practical parameter automation, richer routing/ambient steering,
independent granular layers, improved resampling and shimmer. Each needs bounded
DSP, honest controls and measured target headroom. MIDI, live recording, external
clock and original project compatibility are unsupported; hardware constraints
must guide any future connection work.

Earlier priority lists and comparisons are [archived](../history/research/).
[Release readiness](RELEASE_READINESS.md) owns release gates;
[RESUME](RESUME.md) owns exact next actions and device state.
