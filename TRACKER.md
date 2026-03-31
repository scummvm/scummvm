# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, rechecked `spawn_player_combat_avatar`, `spawn_monster_entity_base`, `spawn_monster_entity_from_record`, and the close-range hit branch in `update_actor_runtime_state` in Ghidra. Confirmed that the player combat avatar has no separate `blood.abm` helper and relies on hit-react banks plus `PC0_hit0/1/2.wav`, while monsters and NPCs do carry a cached `blood.abm` hit effect entity and dedicated monster hit-sound slots (`hit_sound_state_1..3`). Applied the matching engine changes in ScummVM so player hits on monsters now trigger the native blood effect and monster hit sounds, and surviving monsters enter the confirmed hit-react branch with brief horizontal knockback. Verified with `git diff --check` and a successful `make -C build-vscode-harvester-debug -j4 engines/harvester/libharvester.a`.

## Next Suggested Action

- Replay at least one more melee monster besides `DOORKNOB2` and one ranged or special-case encounter, then compare the monster hit blood/sound timing against the corresponding Ghidra runtime states to confirm whether any monsters need per-record hit-trigger handling or different reaction-bank fallback when a model is missing some `0x1c..0x21` hit banks.
