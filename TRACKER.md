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

- Recovered the native `HELP` room-menu branch from both `run_controls_help_screen` and the inline `run_main_menu` help path.
  - Confirmed it alternates between `MOUSHELP.BM` + `MOUSHELP.PAL` and `KEYSHELP.BM` + `KEYSHELP.PAL`, exits on `ESC` or right mouse, and toggles pages on left mouse or any other key.
  - Confirmed the viewer is palette-driven fullscreen help rather than an overlay on the room/menu backdrop, and that it restores `g_current_palette_buffer` before returning to gameplay UI.
- Implemented the engine-side `HELP` screen on the ESC room menu:
  - room-menu `HELP` now loads and displays the two native help pages with their own palettes;
  - page toggling and exit behavior now match the native input flow closely enough for side-by-side validation;
  - returning from help restores the active room palette so the room menu redraw resumes on the correct color state.

## Next Suggested Action

1. Recover the native `SAVE GAME` room-menu path next, including the slot list, description entry, and the pre-save room-state sync point, then compare that against the current ScummVM serializer coverage.
2. If the serializer is still too shallow for real room-state saves, close that gap before wiring the room-menu save UI so the menu does not outpace actual persistence.
