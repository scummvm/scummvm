# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 14, 2026, rechecked native timer handling in Ghidra: `dispatch_room_event_actions` resolves `SET_TIMER`/`KILL_TIMER` through a live class `0x17` timer entity first, mirrors enabled state into `TimerRecord` only when that entity exists, and otherwise only logs `WARNING: Timer %s not in list`; `room_setup` materializes timers only for the resolved room while `destroy_entity_list` preserves global live timer entities across room changes. Tightened the ScummVM fix so missing offscreen live timer entities restore the prior `TimerRecord` instead of being synthesized.

## Next Suggested Action

- Re-run the EYEHALL acid path without depending on `debug_timers`: kill `BIG_EYE`, leave to `MAINHALL`, let `ACID_TRIG_TIMER` start `ACID_TIMER`, `ACID_TIMER2`, and `ACID_TIMER3`, then re-enter EYEHALL and confirm the global acid timers have already decremented instead of restarting from their maximum values.
