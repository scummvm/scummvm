# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, confirmed in Ghidra that native `update_render_entity_screen_position` removes and reinserts actors in `g_render_entity_list` when their z anchor changes, saved that note on `update_actor_runtime_state`, and updated ScummVM player placement to reinsert the player scene entity after accepted movement or combat-loadout placement changes.

## Next Suggested Action

- Re-test `CHESSROOM` movement and keyboard movement through the chessmaster area, confirming the player draws in front at low z/depth while blocker-history logs still avoid repeated synthetic detours before mapping the separate desired-state 0x39/0x3a target-field behavior.
