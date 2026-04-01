# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, traced Harvester room-combat NPC death handling around the `CLOAK_ATND` regression and confirmed combat deaths always finalized through the same removal path used by scripted `KILL_NPC` / `MONSTERFY`, so dead room NPCs were dropped from room materialization instead of persisting as corpses. Added persisted room-NPC corpse runtime state, preserved the last death frame for non-monsterfy combat deaths, and taught room-scene spawning to restore those corpses as non-interactive actors while leaving scripted removals unchanged. Verified with `git diff --check` and a successful `make -C build-vscode-harvester-debug -j4 engines/harvester/libharvester.a`.

## Next Suggested Action

- Reproduce both the `CLOAKROOM` and `EYEHALL` kill-flows across immediate room refresh, room re-entry, disc reload, and save/load to confirm `CLOAK_ATND` now stays behind as a corpse while `BIG_EYE` still remains in the correct defeated state.
