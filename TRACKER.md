# TRACKER

## Current Focus

- Ghidra-backed startup engine alignment from `run_harvester_main_loop`
- Match the native startup room idle/movement behavior before expanding further subsystem coverage
- Keep symbol recovery and engine changes tied to direct script data, call sites, and visible side effects

## Progress

- Program: `HARVEST.LE`
- Total functions: `774`
- Named/documented: `468`
- Still `FUN_*` / undocumented: `306`

## Last Confirmed Action

- Confirmed in Ghidra how native startup-room locomotion is driven.
  - `run_harvester_main_loop()` does not walk to a screen-space `y`; floor clicks store player target `x = g_cursor_screen_x` and player target `z` derived from `g_cursor_screen_y` via the active room's `min_z`, `max_z`, `max_z_screen_y`, and `min_z_screen_y`.
  - Region clicks also target `x/z`, not hotspot-bottom `y`. For class `0x19` regions the native path uses the region center, subtracts `10` pixels when that center is left of the player's current horizontal footprint, and chooses target `z` from the region by desired facing: `0 -> max_z`, `3 -> min_z`, otherwise midpoint.
  - `update_actor_runtime_state()` uses depth-scaled horizontal movement (`round(depth_scale * 8.0f)`) and a separate room-depth step (`z +=/-= 1.0f`, matching the startup room state's `zVelocityStep` default) rather than a flat screen-space speed.
  - `tick_entity_visual_state()` and the actor update path constrain movement through opaque overlap blockers plus z-range overlap; the native startup walk bounds are not script walk boxes.
- Patched the startup stub to follow that recovered model.
  - Startup movement targets now track `x/z`, with screen `y` derived back from room depth when placing the player sprite.
  - Mouse floor clicks, keyboard movement, and pending region transitions now use depth-scaled horizontal stepping, room-depth stepping, and opaque overlap checks against spawned room objects and animations.
  - Region transitions now use the recovered native target placement instead of walking to `region.bottom`, which keeps `PCX1`-style exits aligned with the bedroom/closeup handoff path.

## Next Suggested Action

1. Runtime-test `PCROOM`, `PCX1`, and `PCDRWR` against DOSBox with the new movement path, focusing on spawn placement, walk speed, blocker stops, and the restore point after returning from closeups.
2. Recover more of `update_actor_runtime_state()` if obstacle navigation still differs.
  - The startup stub now matches native target axes, step sizes, and opaque blocking, but it still stops/slide-tests directly instead of reproducing the native blocker-history detour slots at `+0x108c`, `+0x1090`, `+0x1094`, `+0x1098`, and `+0x109c`.
3. Confirm whether startup room records ever override the default `zVelocityStep = 1.0f`; the current startup parser still relies on the struct default rather than a recovered script field.
4. Flesh out the room-menu stub now that `Esc` reaches the correct native entry point, especially the per-item behaviors inside `run_main_menu()` such as resume/save/load/options/quit gating.
5. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
