# TRACKER

## Current Focus

- Recover the native ESC room-menu systems one item at a time from `run_main_menu`
- Keep each menu item implementation grounded in confirmed Ghidra behavior before broadening engine UI helpers
- Extend engine-side persistence only where the native menu paths actually require it

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Recovered and implemented the native `QUIT GAME` room-menu confirmation branch from `run_main_menu`.
  - Confirmed the native dialog is an inline modal over the room/menu backdrop, uses `textbox4.bm` at `(167, 200)`, and pulls its prompt text from `MENU.INI:quitgame` with `/` treated as an explicit line break.
  - Confirmed the native YES/NO hit regions are fixed at `0xd2..0x104,0xee..0x108` and `0x172..0x19a,0xee..0x108`, with keyboard confirm/cancel on `Y` / `N` and cancel also on `ESC` or secondary mouse.
- Implemented the engine-side `QUIT GAME` room-menu flow:
  - room-menu `QUIT GAME` now opens the native-shaped confirmation dialog instead of logging an unimplemented stub;
  - confirm stops startup audio and exits cleanly through the engine quit path, while cancel returns to the room menu without disturbing the active room backdrop or palette.

## Next Suggested Action

1. Run a fresh native-vs-engine visual/input comparison for all four recovered ESC room-menu branches (`OPTIONS`, `HELP`, `SAVE GAME`, `QUIT GAME`) and log any remaining palette, layout, or interaction mismatches.
2. Continue the room/UI pass from the next still-native-only modal or overlay path that shares `run_main_menu` assets and timing behavior.
