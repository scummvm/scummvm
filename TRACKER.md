# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 11, 2026, verified native empty-floor click movement in Ghidra. `run_harvester_main_loop` stores cursor `x` and mapped target depth separately, while `update_actor_runtime_state` satisfies horizontal movement within `depth_scale * 50.0`, satisfies depth within `+/- 8.0`, and lets pending horizontal movement override pending vertical/depth movement. Updated ScummVM target walking to move horizontally first, then vertical/depth, with per-component target latches so frame-width changes do not restart horizontal chasing after it has been satisfied.

## Next Suggested Action

- In-engine, click the empty floor in `CHESSROOM` around `(514,422)` from the logged start position and confirm the player walks right first, then switches to the down-walk/depth movement instead of stopping after the horizontal segment.
