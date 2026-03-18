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

- Recovered the native `OPTIONS` room-menu branch from `run_main_menu` case `3`.
  - Confirmed it loads `options_menu_1..7` from `MENU.INI`, draws three `VOLUME.BM` bars plus three `INDICATR.BM` sliders, previews `WHIP2.WAV` for FX volume, updates music volume through `g_music_stream_state`, and reapplies the current room palette after gamma changes through `update_gamma_brightness_scale`.
  - Confirmed only the `TEXT`, `GORE`, and `PASSWORD` rows append native ` - ...` status suffixes through `format_main_menu_option_status_suffix`.
  - Confirmed the `QUICK TIPS` row reuses the `TIPS.BM` overlay and `ADJHEAD.RCS` text pool, and the password row calls `prompt_for_password` / `run_text_entry_dialog`.
  - Renamed `DAT_000c7e5c` to `g_parental_password_buffer` once the password storage path was bounded from both the options menu and parental-password validation path.
- Implemented the engine-side `OPTIONS` submenu on the ESC room menu:
  - room-menu `OPTIONS` now opens a native-style overlay with working FX / music / gamma sliders, text-mode cycling, gore toggling, quick-tips browsing, and parental-password add/remove behavior;
  - startup config state now tracks `FX_VOLUME`, `MUSIC_VOLUME`, `GAMMA`, `TEXT`, `GORE`, `QUICK_TIPS`, and the parental password buffer, and rewrites sectionless `CONFIG.INI` updates back to the game directory;
  - mixer volume and room/dialogue palette scaling now honor those recovered startup-option values so the submenu changes are live rather than cosmetic.

## Next Suggested Action

1. Trace and implement the native `HELP` room-menu branch next by matching `run_controls_help_screen` / the inline `run_main_menu` help path, including the two-page bitmap toggle and palette restore behavior.
2. After `HELP`, recover the `SAVE GAME` path with the current engine serializer in mind so the room-menu slot UI and actual save-state coverage can be closed together instead of drifting apart.
