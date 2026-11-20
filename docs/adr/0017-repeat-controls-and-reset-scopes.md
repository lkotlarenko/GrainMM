# ADR0017 — Held controls and explicit reset scopes

Date:2026-11-18. Owner requested repeatable D-pad editing and reset options after alpha14.

A portable DirectionalRepeat model handles immediate press,320ms initial delay,
60ms scalar/140ms navigation repeats. Scalar steps become5× after1.5s and10×
after3s; choices remain single-step. Emit at most one repeat/frame, never catch up
in bursts. Ignore duplicate OS keydown; release stops; changing views cancels;
focus loss clears held state. Playback toggles may continue an existing edit.
No callback timing or input handling changes.

ResetScope distinguishes rack-only, rack plus modulation and all sound controls.
The dialog defaults to modulation, lists the three scopes, and requires X.
All-controls pauses before restoring defaults to avoid unexpectedly loud playback.
Sample ownership/path and stored patches remain external. Rack reset still joins
audio, drains stale queued snapshots and clears history; normal MENU saves state.
No preset format change:90fields/settings10 remain. ACTIVE/BYPASS indicates the
selected rack page's parameter state, not a measured global signal/tail detector.

Regression evidence includes wrap/cancel/repeat/acceleration model tests, numerical
reset scopes and actual SDL held-input/reset/save paths. Dummy tests cannot establish
physical ergonomics or audio quality. No additional images are needed for this change.
