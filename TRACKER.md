# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, rechecked the native zero-ammo player-attack path in Ghidra after seeing ScummVM dry-fire still reach the attack-contact log: `update_actor_runtime_state` reads the live ammo/fuel counters inside the player attack branch, calls `consume_weapon_ammo_and_play_fire_or_empty_sound`, and when the count is already `0` it returns before the later contact-resolution block instead of letting the empty shot proceed as a miss. Updated the ScummVM Harvester player attack flow so dry-fire invalidates the pending contact frame/target after the native empty-click path.

## Next Suggested Action

- In-engine, arm and fire the shotgun down to `0` again and confirm the empty-click path no longer emits the `combat player attack contact ...` line at all. If the muzzle-flash frames still appear after that, capture a short video or exact frame sequence: current Ghidra evidence says there is no separate zero-ammo shotgun ABM/loadout path, so any remaining flash would likely be baked into the native attack frames rather than a ScummVM-specific contact bug.
