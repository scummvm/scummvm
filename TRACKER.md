# TRACKER

## Current Focus

- Keep the in-room ESC menu handlers aligned with the native SAVE / OPTIONS layout and font usage
- Verify remaining menu overlays against Ghidra before introducing any more custom UI panels
- Re-test room-menu edge cases after each menu fix instead of bundling multiple visual changes together

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited `run_main_menu`, `run_save_game_menu`, `prompt_for_password`, and `run_text_entry_dialog` in Ghidra and corrected the engine-side SAVE / OPTIONS menu mismatches they exposed.
  - Native SAVE uses `MEDFONT2` for inactive `GAME_##` labels, `MEDFONT1` for the active label and every slot title, and relies on `SAVEGAME.BM` for the lower action-strip art instead of drawing extra `SAVE` / `CANCEL` text.
  - Native save-name editing spawns `TEXT_ENTRY` inline at `(0x50, rowY)` with `g_medfont1_cft`, while password entry overlays the options menu with centered `ENTER PASSWORD` text in `g_harvfont_cft` and `TEXT_ENTRY` at `(0xdc, 0xdc)` in `g_harvfnt2_cft`.
  - The password prompt's hidden max-length register is `8`, not the larger generic limit the engine had been using.
- Updated `startup_menu.cpp` to match those native font/layout rules and rebuilt `engines/harvester/startup_menu.o` successfully.

## Next Suggested Action

1. Re-test the in-room SAVE menu and OPTIONS password prompt in-engine and compare them against DOSBox captures for any remaining cursor or quick-tips overlay drift.
2. If another menu artifact remains, capture the exact submenu state first and then inspect the corresponding `run_main_menu` branch in Ghidra before editing.
