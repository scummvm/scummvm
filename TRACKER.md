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

- Confirmed in Ghidra how native startup inventory transfers work beyond the implicit pickup rule.
  - `run_inventory_screen()` lays out visible `INVENTORY` objects, tracks the selected inventory item state, cancels that state on secondary click, and applies the selected item against either inventory targets or room targets while the inventory panel is open.
  - `add_object_to_inventory()` moves named objects into `INVENTORY` and marks them visible/active there, which is the native backend used by script-driven `ADD2INV`.
  - `g_useitem_records` stores `item_name`, `owner_or_room`, `target_name`, and `action_tag`; matching `USEITEM` records dispatch room event actions, while clicking a pickup-class room target with an item selected still picks the target up first.
- Patched the startup stub to mirror that native behavior.
  - `HARVEST.SCR` parsing now includes `USEITEM`, startup command execution now applies `ADD2INV`, and actionable-command probing treats `ADD2INV` as a real interaction for cursor selection.
  - The startup inventory overlay opens on `I`, keeps the selected item inside the overlay loop, and routes clicks against inventory or room targets through the native pickup-first / `USEITEM` precedence so script-driven transfers can mutate the runtime room and inventory state.

## Next Suggested Action

1. Runtime-test the newly wired startup inventory flows against DOSBox, especially `PC_PEN` on `LODGE_APPLICATION` and `DOLLY` on `PC_CABNET`, and fix any prompt/cursor mismatches that still appear.
2. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
3. Confirm the native side effects of those opcodes in Ghidra before naming any new runtime structures or script helpers.
4. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
5. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
