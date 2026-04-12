# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, refined the Ghidra note for `check_player_region_interaction`: cursor-enabled regions are candidate-selected by cursor/click handling, while cursor-disabled region entities can still act as passive screen/Z overlap triggers. Updated ScummVM post-move activation to sweep only cursor-disabled passive regions and separately test the current hovered region.

## Next Suggested Action

- Re-test `BALLROM1` maintenance warning regions, then `FOYER6_SMOKIN` and `FOYER6_FOYER5`, confirming cursor-disabled passive warnings still fire while cursor-enabled exit regions no longer trigger from unrelated sprite overlap.
