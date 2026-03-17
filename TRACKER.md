# TRACKER

## Current Focus

- Trace the native player-HP path from the inventory portrait back through room-event and actor-runtime state
- Keep health-related engine changes scoped to behavior confirmed in Ghidra
- Only rename health state when the inventory screen, command handlers, and persistence path all agree on the same field or global

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Traced the inventory portrait health path in Ghidra from `run_inventory_screen` back into room-event handling and actor persistence.
  - Renamed `DAT_000d5bb4` to `g_player_current_hit_points` and recovered `RenderEntityRuntime.current_hit_points` at offset `0x1184`.
  - Confirmed the inventory portrait thresholds used by `run_inventory_screen`: `23..30 -> INV_STAT1`, `15..22 -> INV_STAT2`, `8..14 -> INV_STAT3`, `0..7 -> INV_STAT4`.
  - Confirmed `dispatch_room_event_actions` applies `HEAL_PC`, `ADJ_HP`, and `KILL_PC` directly to that HP state, with the healing/adjust path clamped to `30`.
  - Confirmed `run_harvester_main_loop` seeds HP to `30`, `spawn_player_combat_avatar` copies the global into the live actor runtime, `teardown_player_combat_avatar` copies it back out, and the save/load menu code persists the same runtime field.
- Updated the engine inventory/runtime path to match those confirmed behaviors:
  - the startup script now keeps a persistent player HP value and applies native `HEAL_PC`, `ADJ_HP`, and `KILL_PC` mutations to it;
  - the inventory panel now synthesizes the native `INV_STAT1..4` status object from current HP instead of treating those records as ordinary grid items;
  - the health portrait remains non-actionable in the inventory UI;
  - revisiting `run_inventory_screen` showed the native portrait is spawned through `spawn_object_entity_from_record` and drawn by the shared render-entity keyed-blit path, so the engine inventory overlay now uses palette-index-0 transparency instead of an opaque bitmap copy.

## Next Suggested Action

1. Trace where combat or scripted damage outside `dispatch_room_event_actions` decrements `RenderEntityRuntime.current_hit_points`, then decide whether the engine needs a second HP producer beyond the startup-script command path.
2. Confirm whether the adjacent player actor fields around `+0x1188` participate in death-state or portrait-side effects before naming more of `RenderEntityRuntime`.
