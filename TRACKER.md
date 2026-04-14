# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 14, 2026, confirmed in Ghidra that native load restore destroys live render entities, removes preserved global timer entities while freeing old `TimerRecord` nodes, restores saved timer records by name, and lets `room_setup` materialize only timers whose `TimerRecord.arg1` matches the loaded room. Updated ScummVM load-room handoff to clear live scene entities without preserving global timers before applying a pending loaded save.

## Next Suggested Action

- Run with `debug_timers` enabled, start an active global timer in one room, load a save targeting a different room, and confirm the overlay only shows timers materialized for the loaded room.
