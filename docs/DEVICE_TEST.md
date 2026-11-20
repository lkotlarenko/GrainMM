# Device acceptance

Updated 2026-11-20. Candidate **1.0.0-alpha.1** must have its own acceptance.
The last accepted physical checkpoint is **alpha25 / 4f445a4fb862**, with
**1f259d9b5b21** as accepted rollback. Exact selectors remain in
[RESUME](development/RESUME.md); private connection/approval data is never published.

| Area | Accepted alpha25 evidence | New1.0candidate / release gate |
| --- | --- | --- |
| Sound | Owner confirms clean sound, pattern/export and saved settings | New dual/table/sub and groove listening; longer mixed-source soak |
| Controls/display | Owner confirms clear SOURCE/PATTERN and TOOLS/export navigation | Paged SOURCE/GROOVE behavior on physical display/buttons |
| Storage | Pattern/settings17, patch/snapshot/export probes; original WAVs preserved | Schema18 migration/save/reopen, new settings export/resample |
| Runtime |31native silent app cases; software clips0 in retained runs | Target headroom, VNC-off CPU/RSS, scheduling investigation |
| Lifecycle | MENU/save/exit and app-only immutable deployment | Repeated loads/exits, power/sleep/error recovery |

Earlier owner acceptance covers original waveform/patch and effects workflows.
[Alpha25 evidence](history/baselines/ALPHA25_BASELINE.md) and
[earlier history](history/DEVICE_TEST_THROUGH_ALPHA9.md) preserve their conditions.
No old measurement is presented as new workload evidence.

Host/ARM/QEMU/dummy-audio results do not establish physical listening. Backend
XRUN counts are unavailable; callback lateness is only a scheduling hint.
Headphone/speaker, longer stress, power/sleep and VNC-off profiling remain separate.
See [new validation](development/V1_VALIDATION.md) and
[release readiness](development/RELEASE_READINESS.md). The owner authorized the experimental prerelease; stable acceptance remains gated.
