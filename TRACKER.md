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

- Confirmed in Ghidra how native startup room exits and entrance placement are wired.
  - `room_setup()` resolves the requested `ENTRANCE` record and applies its three integer fields directly as player `x`, `y`, and `z`, using `set_entity_screen_position()` with `(entrance_x - frame_x_offset - width / 2, entrance_y - height - frame_y_offset, entrance_z)` before and after `rescale_entity_sprite_for_depth()`.
  - The `START` and `PC_ROOM_HOUSE_START` entries in `HARVEST.SCR` both point to `PCROOM` at `(360,405,15)` facing `LEFT`, so native room setup does not add any extra centering offset beyond the entrance record itself.
  - `PCDOOR` is just a mask object with interaction label `NULL_ID`; the actual bedroom exit is `REGION "PCX1"` with command tag `PCEXIT1`, and `run_harvester_main_loop()` only gives class `0x19` regions cursor sequence `6` with no prompt text.
  - `check_player_region_interaction()` requires screen-rect overlap, z-range overlap, a matching facing, and `start_enabled != 0` before dispatching the region command chain.
- Patched the startup stub to mirror that native split.
  - `HARVEST.SCR` `REGION` records are now parsed into the startup script layer and materialized into room state.
  - Startup room hover/click now prefers active regions over neutral `NULL_ID` masks, so `PCDOOR` no longer fabricates a prompt while `PCX1` drives the transition cursor and room-change command path.
  - The startup room loop now keeps a pending region target, waits for player overlap plus facing, and only then dispatches the region command chain, matching the native closeup/door transition shape more closely.

## Next Suggested Action

1. Runtime-test `PCROOM` against DOSBox with the new region path, especially the `PCDOOR` hover, the `PCX1` transition cursor, and the player restore point after returning from closeups.
2. Compare startup player placement visually in `PCROOM` after confirming the native `START` coordinates above; if it still reads too far left, the remaining mismatch is in runtime sprite metrics or render anchoring rather than script-room setup.
3. Revisit native locomotion in `update_actor_runtime_state()` and the main loop before changing movement speed or walk bounds.
  - The current startup stub still uses direct screen-space interpolation and does not mirror the native movement-state machine, blocker history, or z-aware overlap tests.
  - Any further speed or walk-area changes should come from that recovered actor-state path, not from ad hoc constants.
4. Flesh out the room-menu stub now that `Esc` reaches the correct native entry point, especially the per-item behaviors inside `run_main_menu()` such as resume/save/load/options/quit gating.
5. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
