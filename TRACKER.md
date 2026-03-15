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

- Confirmed in Ghidra that `room_setup` dispatches the current room's `on_exit_tag` before resolving and loading the destination room.
  - Startup exit tags such as `CEM2_EXITCOM`, `CEM10_EXITCOM0`, and `ARRESTED_EXITCOM` were decoded from `HARVEST.SCR`.
  - The confirmed startup exit opcode mix is broader than audio alone: `DELETE_WAV`, `SET_FLAG`, `SET_NPC`, `SET_TIMER`, `KILL_TIMER`, `SET_ANIM`, `ADD`, `DELETE`, and `CHECK_FLAG` all appear on room-exit chains.
- Patched the startup stub to mirror the confirmed room-exit audio cleanup path.
  - `resolveRoomSetupState()` now collects room-exit audio commands alongside room-enter audio commands.
  - `runRoomLoop()` now executes those exit audio commands before recursive room handoff and again when a stub room loop closes, so startup `DELETE_WAV` cleanup no longer depends on the global `stopStartupSound()` fallback.
  - Parent rooms now reapply their room-enter audio setup after a nested room loop unwinds, preserving the current recursive-room stub behavior.

## Next Suggested Action

1. Introduce persistent startup script runtime state so `SET_FLAG`, `ADD`, and `DELETE` effects from room-enter, room-exit, and object-interaction chains survive across room transitions instead of being recomputed from `_flags` and `_objects` defaults.
2. Extend startup exit handling beyond audio once that state exists, starting with the confirmed room-exit opcodes already present in startup rooms: `SET_NPC`, `SET_TIMER`, `KILL_TIMER`, and `SET_ANIM`.
3. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
4. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
