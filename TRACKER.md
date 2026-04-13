# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, fixed CHESSKING combat startup rendering in ScummVM: `PC_GOTO_XZ` now maps the requested Z depth back to the room screen Y before placing the player, and monster combat animation selection now rejects fully transparent placeholder frame banks so `KING.ABM` does not choose blank attack/death ranges.

## Next Suggested Action

- Re-test the Chessmaster fight path: choose "No...", confirm `PC_GOTO_XZ applied` places the player near CHESSKING's depth line, monster attacks use drawable frame ranges `80..89` or `110..119` instead of blank `90..109`/`120..139`, and projectile death uses a visible death bank rather than corpse frame `295`.
