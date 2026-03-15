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

- Confirmed in Ghidra that the native startup path keeps mutable town-script runtime state beyond room-local audio.
  - `initialize_town_script_runtime` is called from the startup path before `room_setup`, which aligns with the native `ADD`/`DELETE` and `SET_ANIM` behavior observed in decoded `HARVEST.SCR`.
  - Native room transitions still dispatch the current room's `on_exit_tag` before the destination room is resolved and loaded.
  - Decoded startup exit chains such as `CEM10_EXITCOM0`, `DNALFT_EXITCOM`, and `SCH_HALL_EXITCOM` include `SET_FLAG`, `SET_NPC`, `SET_TIMER`, `KILL_TIMER`, `SET_ANIM`, `ADD`, `DELETE`, `DELETE_WAV`, and `CHECK_FLAG`.
- Patched the startup stub to persist the runtime state that the current engine can model directly.
  - `StartupScript` now keeps mutable runtime copies of flags, objects, and animations instead of rebuilding them from `_flags`, `_objects`, and `_animations` on every query.
  - `resolveRoomSetupState()` mutates that runtime state through room-enter commands, `executeRoomExitCommands()` now applies room-exit chains at handoff time, and `materializeRoomState()` rebuilds the current room from live runtime state without re-running room-enter commands.
  - `runRoomLoop()` now refreshes the current room from runtime state after same-room command mutations and after nested room unwind, preserving the player's current placement while carrying forward `SET_FLAG`, `ADD`, `DELETE`, and `SET_ANIM`.

## Next Suggested Action

1. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
2. Confirm the native side effects of those opcodes in Ghidra before naming any new runtime structures or script helpers.
3. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
4. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
