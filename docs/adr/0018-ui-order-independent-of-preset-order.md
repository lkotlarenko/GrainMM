# ADR0018 — UI order independent of persisted parameter order

Date:2026-11-18. Owner reports inconsistent MIX placement and navigation.

Keep PageDefinition.fields in immutable codec order; add a six-index display_order
permutation with identity default. The editor resolves its labels, choices, bounds,
values, adjustment and selected-field reset through that mapping. Serialization
continues to access the original fields directly. Compile-time validation requires
every index once. This adds no callback work, allocations or audio format changes.

Standard policy: eight wet/dry effects place MIX sixth (bottom-right). Output or
makeup gain, when present, is fifth (bottom-left). Primary shaping occupies the
first row, remaining shaping/modulation the second. EQ remains frequency-ordered;
RHYTHM has two independent depths, not a fictitious MIX. Non-effects pages retain
musically meaningful source/trim/oscillator controls. Do not invent missing controls
to make dissimilar algorithms look identical.

Keep D-pad selection/edit/repeat, A play/pause, MENU save/exit and current reset
chord. The owner's video transcript describes different transport/edit shortcuts;
those require a separate coherent input design and are not silently substituted.
Existing patches/settings remain schema10, without reinterpreting any values.
Numerical editor tests assert common positions and editing/reset behavior; codec
legacy goldens remain independent. SDL probes navigate the actual new positions.
