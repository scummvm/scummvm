# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 14, 2026, fixed ScummVM Harvester global timer runtime handling so offscreen global timers touched by `SET_TIMER`/`KILL_TIMER` are synchronized as hidden timer entities after non-visual script mutations, including command chains that immediately transition rooms. Timer reconfiguration now also honors the entity manager pause depth.

## Next Suggested Action

- Re-run the EYEHALL acid path without depending on `debug_timers`: kill `BIG_EYE`, leave to `MAINHALL`, let `ACID_TRIG_TIMER` start `ACID_TIMER`, `ACID_TIMER2`, and `ACID_TIMER3`, then re-enter EYEHALL and confirm the global acid timers have already decremented instead of restarting from their maximum values.
