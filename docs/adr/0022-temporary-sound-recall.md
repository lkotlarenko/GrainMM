# ADR0022 — Source-safe temporary sound snapshots

2026-11-19. The post-alpha18 report prioritizes safe experimentation before adding
more engines. A single temporary snapshot supports immediate A/B sound decisions
with no SD writes, sample copies or extra audio voices.

`SoundSnapshot` owns a bounded `InstrumentParameters` value and source-generation
number on the control thread. It owns no sample, path, effect history, transport,
UI preference or project file. Every successful source replacement increments the
generation, including reloading the same path. Empty or changed-source recall
leaves the editor/audio unchanged and reports why. A failed source load does not
advance the generation or invalidate recall. Re-capture intentionally replaces
the temporary slot; the slot does not survive exiting the app.

On the main parameter view, hold SELECT first and tap A to capture, or B to recall.
Consuming this modifier suppresses the deferred help tap and reset chord until
release. Debounced A/B do not toggle transport/open trim, and the chord is inactive
in modal views, waveform, help and reset confirmation. MENU still exits. Ordinary
SELECT help and held START+SELECT reset retain their behavior. No reverse-order
chord is promised: pressing A first still performs its ordinary play/pause action.

Recall restores editor values and uses the existing prepared-parameter queue,
retaining its bounded backpressure/retry and transition behavior. It marks musical
settings dirty for normal MENU autosave. Capture alone never marks settings dirty.
Effect tails, grain positions, collision trajectories and clock phase continue;
this is settings recall, not a recording or rewind of the audio engine. Playback
state and selected WAV never change behind the gesture.

The existing one-line message area shows captured/recalled/empty/source-changed
feedback for2.4seconds, then returns to startup diagnostics. Help describes the
main-view gesture. No image inspection is needed for text in the existing bounded
banner. Tests cover full-value copy/ownership/bounds, rejected recall, recapture,
modifier cancellation/rearm/reset/focus loss, and actual SDL key sequences with
playing and paused recall, help/transport suppression and source replacement.
Settings12/108fields, patches and all audio algorithms are unchanged.
