# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 11, 2026, verified native blocked click-path handling in Ghidra. `update_actor_runtime_state` pushes perpendicular waypoints around the current blocker: horizontal blockers create a Z detour from the blocker depth span, vertical/depth blockers create an X detour from the blocker screen span, and directional blocker-history slots let the detour proceed past the same blocker. Updated ScummVM click walking to queue those detours instead of clearing the target on the first blocked step.

## Next Suggested Action

- In-engine, click the empty floor in `CHESSROOM` around `(494,415)` from the `FOYER7_2_CHESS` spawn and confirm the player walks right, queues a depth detour around the room blocker, then resumes toward the clicked floor position.
