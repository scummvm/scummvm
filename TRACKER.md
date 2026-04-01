# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, rechecked the native inventory/combat HUD path in Ghidra before wiring ScummVM's Harvester inventory toggle feedback: `run_inventory_screen` at `0x7df10` removes any visible combat-resource strip, formats `Disarming %s...` or loadout-specific `Arming %s, %i ...` text inside `BOX1`, then calls `set_player_combat_loadout`, while `run_harvester_main_loop` pre-spawns the ammo/fuel icon strips at `x = 10 + 15 * i`, `y = 30` with capacities `16/16/8/6/16` for nails, shells, `9GUN` bullets, `.38` bullets, and gas. Updated the ScummVM Harvester runtime to preserve those combat resource counts, execute the confirmed `ADD_SHOTGUN_SHELLS` / `ADD_NAILS` / `ADD_GASCANS` script opcodes, draw the matching HUD strip, and show the native-style arming/disarming status box on inventory weapon toggles.

## Next Suggested Action

- Runtime-smoke the inventory combat path with shells in hand: right-click `SHOTSHELL` to run `ADD12SHELLS`, right-click `SHOTGUN`, and confirm the room view now shows both the `BOX1` arming/disarming message and a shell strip at the native top-left HUD position. If the shell count still fails to change after `ADD12SHELLS`, capture the corresponding `Harvester: action tag` / `inventory secondary action` log lines so we can compare the command chain against native `dispatch_room_event_actions`.
