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

- Confirmed in Ghidra how native startup closeups and pickup hotspots differ from ordinary object interactions.
  - `spawn_object_entity_from_record()` drives pickup cursor state from the object record's alternate inventory sprite path, while `run_harvester_main_loop()` uses that flag to select the native `Pick up the %s` prompt instead of `Operate the %s`.
  - The native click path moves pickup targets into `INVENTORY` by changing the object record's owner, not by toggling visibility.
  - `room_setup()` injects the global `EXIT_BM` and `EXIT_HS` objects whenever the room is entered in closeup mode, which explains the exit sign shown over rooms such as `PCDRWR`.
- Patched the startup stub to mirror that native behavior.
  - Startup pickup targets now use the pickup cursor, move into `INVENTORY` on click, and rebuild the current room from runtime state so items such as `PC_PEN` and `QUARTER` disappear from the closeup after pickup.
  - Closeup rooms without an entrance now materialize the global exit sign/hotspot pair, and clicking that exit unwinds back to the parent room through the existing recursive room-loop restore path so the player returns to the coordinates they had before entering the closeup.

## Next Suggested Action

1. Confirm and implement the remaining native inventory transfer paths exposed in `HARVEST.SCR`, especially script-driven `ADD2INV` and `USEITEM` flows that go beyond the implicit pickup rule.
2. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
3. Confirm the native side effects of those opcodes in Ghidra before naming any new runtime structures or script helpers.
4. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
5. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
