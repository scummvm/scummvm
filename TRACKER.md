# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, added a ScummVM `RoomSystem` projectile contact fallback based on Ghidra `update_actor_runtime_state`: when a projectile attack reaches its contact frame with no captured cursor target, it scans active visible room monsters using live Z-overlap and attack-direction geometry, assigns the selected monster as the attack target, and then applies the existing monster damage path.

## Next Suggested Action

- Re-test shotgun attacks against `BURNTCREATURE` in `SMOKING`: confirm empty-cursor shots log `combat player projectile contact fallback target='BURNTCREATURE'`, then `combat player monster hit`, and that shells still dry-fire once depleted.
