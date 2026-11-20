# Remote debugging and deployment

Read RESUME.md before device actions; use existing evidence before repeating probes.
Device access and ongoing app-only testing were explicitly approved. Scope:
/mnt/SDCARD/App/GrainMM, including state. No writes to Saves/Onion internals,
service/configuration changes, critical-process termination or reboot are approved.
Credentials, endpoints, SSH control sockets, approval records and captures live
only under ignored .local/hardware. Authenticate interactively; never store passwords.

## Verified SSH compatibility

On this Onion target, a per-connection `-o MACs=hmac-sha1` allowed interactive
login after default-MAC authentication drops. Use it only for this compatibility
case; do not weaken server settings or store passwords. Connection details remain
private. Earlier transfer evidence does not establish new candidate deployment or listening.

## Tools

- scripts/miyoo/remote.py: reviewed bounded script over an existing authenticated SSH control session.
- scripts/miyoo/vnc_capture.py: RFB3.8 shared raw capture, None security on existing private service; defaults to a single requested frame.
- `--key-only --key ...`: input with **zero screenshots**; bounded arrow repetition only.
- VNC event journal and navigation map are private; use them instead of repeated image analysis.
- scripts/miyoo/deploy.py: default read-only remote preflight and host staging; --apply requires recorded owner approval. It verifies archive/ELF/checksums, rejects active GrainMM executables, foreign roots, symlinks and collisions.
- Immutable releases and CURRENT/PREVIOUS selectors avoid overwriting executables. LAST_GOOD is a separately smoke-verified rollback target; PREVIOUS alone may be a failed experiment.
- `--rollback` selects LAST_GOOD, or an explicitly reviewed --rollback-release. No builds/state are deleted. Unknown/bootstrap changes require review; failed staging remains for inspection.

Existing VNC was verified as RFB3.8/None,640x480, shared raw frames; real menu
movement worked and native app frames were visible. The user reduced server FPS
from15 to2 through Tweaks. Do not change services/network settings automatically.
The server remains user-configured at2FPS. No internet exposure/tunnels were created.
Onion reference: https://onionui.github.io/docs/network/vnc .

At low FPS, a frame requested during launch may contain an old or transitional
screen. Check process/startup logs first. Image inspection is reserved for a
critical first UI validation or unresolved rendering bug; reuse a captured frame.
Never use a screenshot for every navigation key. Physical confirmations remain
separate from VNC observations. Tests without VNC are still pending.

A stale SSH multiplexed session stopped responding while fresh SSH/VNC ports
remained reachable. Reconnecting only our host client restored access; no device
service was restarted. Future sessions should use short keepalive/timeouts. Avoid
retrying mutations blindly after a transport failure; inspect CURRENT and hashes.

SSH environment reference: https://onionui.github.io/docs/network/ssh .
Historical default negotiation worked in an earlier session; the later compatibility
case required the per-connection option above. Never change server configuration.

## Script transport and SD retention

Use remote.py's stdin transport for reviewed scripts; large SSH exec requests
were associated with connection resets on this target. This avoids putting the
entire script in remote command arguments and passed the same long cleanup probe.
A host regression executes a32KiB-plus quoted script through stdin. Do not change
SSH services/authentication or keep retrying a write without inspecting its state.

Keep build archives/log evidence on the host. After a candidate is verified, keep
one current app and one verified rollback on SD. Verify obsolete file hashes
against host archives before exact-file removal; refuse symlinks/running apps.
Never use recursive broad cleanup, never prune user state, and never delete an
unrecognized file just to remove a directory. A partially completed cleanup must
be inspected before retry. The original440-file reference snapshot stays on host.

## Reusable UI documentation captures

`display_test` exercises the actual SDL2 software renderer and checks redraw-cache
invalidation on selection/value/page/error/browser transitions. It captures only
when given a **new** host output directory, with synthetic filenames and no audio:

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy build/host-debug/display_test build/ui-preview-NEW
magick build/ui-preview-NEW/grain.bmp -strip docs/images/grain-controls.png
magick build/ui-preview-NEW/browser.bmp -strip docs/images/sample-browser.png
```

BMP capture exclusively creates files; an existing destination is refused. Review
screenshots only for a new layout or rendering fault and reuse them afterwards.
README images are these original host UI fixtures, explicitly labeled; they do
not establish physical orientation or framebuffer correctness. Private device
captures and navigation remain ignored and must never be copied into docs.
The main screen now shows all six actual page controls; callback diagnostics
remain available in session logs instead of occupying the instrument interface.
