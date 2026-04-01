# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, rechecked the native room-action dispatcher in Ghidra before touching the remaining `SET_MONSTER` path: `dispatch_room_event_actions` at `0x60ee0` resolves monster records by `monster_name`, then gates the live spawn/remove work on `record->room_name == g_current_room_def->room_name`. Updated the ScummVM Harvester action-dispatch paths to carry explicit current-room context into room entry/exit commands, continuations, timers, pickup actions, inventory secondary actions, and monster/NPC death tags so `SET_MONSTER` now prefers the current-room runtime record before falling back to name-only resolution. Verified with a successful `make -j4 engines/harvester/script.o engines/harvester/room.o`.

## Next Suggested Action

- Runtime-smoke the exact regression path again: kill the CRYPT `WOLFIE`, transition into DINING, and confirm its `WOLFIE` materializes alive with full hit points. If the issue still reproduces, inspect the remaining room-agnostic `executeActionTag` callers outside the room loop (for example dialogue helper files) and confirm whether any duplicate `WOLFIE` action tags still enter the dispatcher without an explicit room context.
