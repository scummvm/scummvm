# TRACKER

## Current Focus

- Match room and inventory palette presentation to the native VGA DAC upload path
- Keep palette changes scoped to confirmed Ghidra behavior instead of screenshot-only tuning
- Distinguish true native defaults from user config settings before changing brightness or gamma handling

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
- Revisited the palette/brightness path in Ghidra after comparing DOSBox and engine captures.
  - Confirmed the native `CONFIG.INI` in the installed game currently has `GAMMA=0`, so the washed-out comparison is not explained by a non-default gamma setting that DOSBox applies and the engine ignores.
  - Confirmed `update_gamma_brightness_scale` sets the native gamma scalar to `1.0` when `g_gamma_level_index <= 0`, and `upload_palette_to_vga` / `upload_palette_to_vga_unscaled` always quantize stored `.PAL` bytes down to VGA DAC `0..63` before display.
  - The engine was uploading room and wait palettes as raw 8-bit RGB, so it skipped that DAC quantization step. Updated the engine palette path to emulate the native VGA upload rule before applying room brightness.

## Next Suggested Action

1. Re-test the same inventory scene against DOSBox after the DAC-quantized palette change to see whether the remaining mismatch is explained by palette upload alone or by a later post-processing/display difference.
2. If the image is still visibly off, trace whether DOSBox capture differences come from a second native layer such as menu fade timing, scaler/aspect treatment, or platform-side palette handling rather than from Harvester’s own palette math.
