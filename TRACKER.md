# TRACKER

## Current Focus

- Ghidra-backed startup engine alignment from `run_harvester_main_loop`
- Use the recovered render/entity runtime to remove raw actor offsets from the remaining startup/combat movement paths
- Keep symbol recovery and engine changes tied to direct script data, call sites, and visible side effects

## Progress

- Program: `HARVEST.LE`
- Total functions: `774`
- Named/documented: `468`
- Still `FUN_*` / undocumented: `306`

## Last Confirmed Action

- Recovered the shared render/entity runtime layout in Ghidra around `spawn_scaled_abm_entity_from_resource()`, `attach_abm_resource_to_entity()`, `hide_entity_visual()`, `tick_entity_visual_state()`, and `update_actor_runtime_state()`.
  - Confirmed helper types:
    - `BitmapBuffer` is the 12-byte runtime bitmap header `{ width, height, pixels }`.
    - `AbmFrameHeader` is the packed ABM frame header prefix `{ x_offset, y_offset, width, height, compressed_flag, encoded_size }`.
    - `DirtyRectNode` is the 20-byte dirty-rect list node `{ left, top, right, bottom, next }`.
    - `ActorWaypoint[10]` is the 10-entry actor waypoint array at `RenderEntityRuntime + 0x10ac`.
  - Confirmed `RenderEntityRuntime` fields now typed in Ghidra include:
    - bitmap pointers and render placement at `+0x00 .. +0x48`
    - animation timing/state at `+0x105c .. +0x1084`
    - directional blocker history slots at `+0x108c`, `+0x1090`, `+0x1094`, `+0x1098`
    - the current opaque overlap blocker at `+0x109c`
    - movement/waypoint state at `+0x10a0 .. +0x1124`
  - Switching the simple entity helpers to Borland `__fastcall` made the entity pointer resolve correctly in decompilation, which in turn exposed the blocker-history detour logic directly inside `update_actor_runtime_state()`.
- No engine code changed in this pass.
  - This was a Ghidra-first recovery pass to establish the runtime structure before any further startup/combat movement changes.

## Next Suggested Action

1. Recover `update_render_entity_screen_position()` and `set_entity_screen_position()` precisely enough to eliminate the remaining raw screen-anchor/dirty-rect offsets.
  - The helper cluster is now on the right shared runtime struct, but those position-update helpers still have calling/storage oddities that prevent the decompiler from expressing all arguments cleanly.
2. Continue typing the actor-only tail of `RenderEntityRuntime` from `+0x1134` through `+0x118e`.
  - That range now clearly holds combat/use-state sample slots, attack timing, and facing-bank flags, and typing it would clean up both combat logic and the remaining locomotion detour states.
3. Runtime-test `PCROOM`, `PCX1`, and `PCDRWR` against DOSBox with the recovered blocker-history model in mind.
  - The startup stub can now be compared against confirmed native detour states instead of guessed movement constants.
4. Confirm whether startup room records ever override the default `zVelocityStep = 1.0f`; the current startup parser still relies on the struct default rather than a recovered script field.
5. Flesh out the room-menu stub now that `Esc` reaches the correct native entry point, especially the per-item behaviors inside `run_main_menu()` such as resume/save/load/options/quit gating.
