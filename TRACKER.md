# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 11, 2026, verified native monster room-spawn ordering in Ghidra. `spawn_monster_entity_from_record` restores a saved monster corpse frame from the record and advances the entity frame before calling `set_entity_screen_position`. Updated ScummVM room population to apply monster animation state before calculating actor placement, so re-entered monster corpses start in the correct screen position.

## Next Suggested Action

- In-engine, re-enter `BALLROM1` from `BALLROM3_2_BALLROM1` after killing `MAINT_MAN_MNST` and confirm the corpse is drawn at its final location immediately, without a first-frame shift.
