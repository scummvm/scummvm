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

- Confirmed in Ghidra how the native startup room loop reaches both the game menu and the inventory screen.
  - `run_harvester_main_loop()` routes `g_last_keyboard_scancode == 1` (`Esc`) into `run_main_menu()`, not a room-exit path, and `run_main_menu()` reads the same `main_menu_1..6` entries from `MENU.INI` that the startup stub already loads.
  - The same loop routes `g_last_keyboard_scancode == 0x17` (`I`) into `run_inventory_screen()`, and it also calls `run_inventory_screen()` when the clicked entity is class `5` and the player avatar animation is idle/complete.
  - In the native hover scan, entity class `5` uses cursor sequence `1`, which matches a player-avatar hotspot rather than a walk target or ordinary room object.
- Patched the startup stub to mirror that native behavior.
  - The startup player actor now has a dedicated hit-testable player class so clicking directly on the avatar opens the inventory overlay only when the actor is idle, matching the native class-`5` gate.
  - `Esc` in the startup room loop now opens a dedicated room-menu stub backed by the existing `MENU.INI` entries instead of incorrectly unwinding the room loop as if it were a closeup exit.

## Next Suggested Action

1. Runtime-test the startup room menu and player-click inventory entry against DOSBox, and adjust prompt/cursor behavior if the player hotspot needs a native text label rather than just the cursor change.
2. Flesh out the room-menu stub now that `Esc` reaches the correct native entry point, especially the per-item behaviors inside `run_main_menu()` such as resume/save/load/options/quit gating.
3. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
4. Confirm the native side effects of those opcodes in Ghidra before naming any new runtime structures or script helpers.
5. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
6. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
