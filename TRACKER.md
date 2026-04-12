# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, used the `CHESSROOM` pathfinding logs to refine blocker avoidance again: detour waypoints now sample their route against other room blockers before being accepted, and horizontal movement can fall back to a side waypoint when a depth detour would immediately collide with another blocker.

## Next Suggested Action

- Re-test the `CHESSROOM` click toward `(464,422)` with `--debugflags=pathfinding` and `DEBUG_PATHFINDING` enabled, checking for `detour candidate blocked` lines and confirming the player routes around `__ANON_OBJECT_35` without being sent back into `__ANON_OBJECT_33`.
