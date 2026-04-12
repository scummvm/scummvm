# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, checked native `update_actor_runtime_state` waypoint handling in Ghidra and refined ScummVM player blocker avoidance so detour waypoints must clear the blocker span and use tighter completion slack than ordinary click targets.

## Next Suggested Action

- Re-test the `CHESSROOM` click toward `(457,410)` with `--debugflags=pathfinding` and `DEBUG_PATHFINDING` enabled, confirming the player reaches the front-side z detour instead of alternating between blocker-depth waypoints.
