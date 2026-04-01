# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, rechecked the native player-attack and weapon-toggle paths in Ghidra after seeing infinite shotgun shells and a persistent inventory overlay in ScummVM: `update_actor_runtime_state` calls `consume_weapon_ammo_and_play_fire_or_empty_sound` on the player attack's first-frame-plus-one commit for loadouts `2/3/4/5/0xe`, so ranged/fuel attacks spend resource before the later contact frame, and `run_inventory_screen` exits its loop immediately after the `BOX1` arming/disarming message path tears down. Updated the ScummVM Harvester room/player/script flow so resource-backed attacks consume one unit from the persisted HUD counts on each committed shot and weapon toggles close inventory after the native-style status message finishes.

## Next Suggested Action

- In-engine, arm and fire the shotgun until empty, confirming the on-screen shell strip drops by `1` on every committed shot including misses and that the final dry-fire no longer lands damage; then right-click arm and disarm a weapon from inventory and confirm the panel closes as soon as the `BOX1` status message finishes. If either behavior still disagrees, capture the next `combat player attack start/contact` lines plus the next `PLAYER_ATTACK player combat resource ...` line and the surrounding `inventory combat toggle` lines.
