# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, checked `HARVEST.LE` monster record handling in Ghidra and updated ScummVM Harvester monster runtime matching so parsed monster records keep a stable `recordIndex`; room-qualified monster sync no longer falls back to the first same-name record, preventing a dead duplicate like `WOLFIE` from contaminating another room's monster state.

## Next Suggested Action

- Re-run the duplicate `WOLFIE` path with `--debugflags=room,combat` from a clean pre-kill state and confirm the DINING monster materializes with its own `hp=20/20`, `spawned=0`, and `runtimeState=-1` unless the DINING record itself has been killed.
