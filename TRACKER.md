# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, verified native repeated `SET_TIMER ... ON` behavior in `HARVEST.LE` and patched ScummVM so an already-enabled global timer keeps its live countdown when a room-entry command enables it again; preserved live global timers are no longer reconfigured only because the script record's saved `currentValue` is stale.

## Next Suggested Action

- Re-run the Dining/Mainhall acid timer path with `DEBUG_TIMERS`: start the global acid timers, leave and re-enter Mainhall, and confirm `ACID_TIMER2` / `ACID_TIMER3` keep counting instead of resetting.
