# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, rechecked Ghidra `update_actor_runtime_state` class-6 pursuit and confirmed native horizontal pursuit stores a target frame-left X derived from the player's edge, the MONSTER engage field, and monster frame width. Updated ScummVM `RoomSystem` monster chase logic to use that native frame-left waypoint instead of the previous center target, which could leave `BURNTCREATURE` stationary inside the 50-pixel waypoint tolerance while still outside engage range.

## Next Suggested Action

- Re-test entering `SMOKING` from `BAKSTAG2`: confirm `BURNTCREATURE` emits immediate `combat monster chase move` logs while outside `engage=100`, keeps advancing on animation ticks, and still starts attacks once `abs(live_center_dx) <= 100`.
