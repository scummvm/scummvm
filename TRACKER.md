# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, verified native `SET_TIMER` / `KILL_TIMER` and class `0x17` timer persistence behavior in `HARVEST.LE`: `g_timer_records` is the global timer record list, `room_setup` only spawns timers whose `arg1` matches the resolved room, and global timer persistence is implemented by `destroy_entity_list` preserving live timer entities whose copied global flag is set. Patched ScummVM to preserve live global timer entities across room scene clears, avoid duplicate timer spawns on return to the owning room, and make timer debug/runtime sync paths include preserved live timers.

## Next Suggested Action

- Re-run a script path that starts a global timer, leave its owning room, enable `DEBUG_TIMERS`, and confirm the live timer remains visible/counting across the transition and still fires its `TimerRecord.arg2` action.
