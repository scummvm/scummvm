# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, validated the native room-walk update path in Ghidra: `spawn_player_combat_avatar` seeds player runtime field `+0x1140` with `1`, and `update_actor_runtime_state` copies `abs(+0x1140)` into `frame_y_step` for vertical walk states while applying `+/- g_current_room_def->z_velocity_step` separately through `frame_z_step` before `set_entity_screen_position` commits `screen_y` and `z` independently. Updated the ScummVM Harvester room player movement to keep `bottomY` as live state, step vertical movement in screen space, and preserve explicit move-target Y instead of remapping it from `z` every tick. Verified with a successful `make -j4 engines/harvester/player.o engines/harvester/room.o engines/harvester/flow.o engines/harvester/script.o`.

## Next Suggested Action

- Runtime-smoke BAR room movement against the original DOS binary: compare keyboard up/down stepping, floor-click move targets near the walk band edges, and region/entrance transitions (`BAR_DINING`, `BAR_FOUNT`, `DININGROM_2_BAR`, `FOUNTAIN_2_BAR`) to confirm the new independent `bottomY` tracking matches native room walking and no longer snaps between `y=393` and `y=332`.
