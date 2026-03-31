# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, rechecked `spawn_player_combat_avatar`, `update_actor_runtime_state`, and `update_player_combat_avatar_state` in Ghidra and confirmed the native room-combat hit path: monsters enter attack states `0x16..0x1b` (`0x50..0x8b` frame banks), a landed hit assigns player knockback through `remaining_knockback_x`/`knockback_x_decay_step` (`+0x11ac`/`+0x11b0`), switches the player into hit-react states `0x1c..0x21` (`0x8c..0x9d`), and plays one of `PC0_hit0/1/2.wav`. Applied the matching engine changes in ScummVM so monster melee uses the native attack banks and the player now enters the confirmed hit-reaction animation/sound path with horizontal recoil. Verified with `git diff --check` and a successful `make -C build-vscode-harvester-debug -j4 engines/harvester/libharvester.a`.

## Next Suggested Action

- Replay at least one more melee monster besides `DOORKNOB2` and one projectile-capable encounter against the updated engine, then compare the animation/sound timing against the corresponding Ghidra states to confirm whether any monster-specific contact offsets or player hit/death edge cases still need native-special handling.
