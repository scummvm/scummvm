# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, rechecked native `update_actor_runtime_state` and `run_harvester_main_loop` in Ghidra, then removed non-native route sampling, alternate detour candidates, clamped avoidance targets, and tighter avoidance-only waypoint slack from the ScummVM player pathfinding code.

## Next Suggested Action

- Re-test `CHESSROOM` movement with `--debugflags=pathfinding` and `DEBUG_PATHFINDING` enabled, comparing each emitted detour against Ghidra's native rule: horizontal blocks insert one depth waypoint from the blocker z span, and depth blocks insert one horizontal waypoint beside the blocker.
