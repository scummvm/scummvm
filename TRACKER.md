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

- Rechecked the native `OPTIONS` room-menu draw order in `run_main_menu` to explain the slider-row text mismatch from the visual comparison.
  - Confirmed all seven option text entities are spawned before the three `VOLUME.BM` bars and three `INDICATR.BM` sliders are added to the render list, so the top three labels are partially covered where they overlap the slider art.
- Adjusted the engine-side `OPTIONS` renderer to match that native layering.
  - `SOUND FX`, `MUSIC`, and `GAMMA` now draw underneath the volume bars and indicators instead of painting on top of them, which restores the native-looking spacing against the sliders.

## Next Suggested Action

1. Re-run the native-vs-engine visual comparison for the `OPTIONS` submenu with the same selected row in both captures to confirm the slider-row spacing now matches.
2. Continue the remaining room/UI visual-parity pass, especially any menus where native render-list ordering still differs from the engine's direct blit order.
