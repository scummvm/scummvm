# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, confirmed that reused monster names can alias runtime combat state across rooms: Harvester matched `_currentMonsters` and base monster definitions by `monsterName` alone during town reload, save migration, runtime sync, and NPC monsterfy resolution, so killing `WOLFIE` in one room could materialize a different-room `WOLFIE` as an already-dead corpse. Updated Harvester script runtime monster matching to prefer `(roomName, monsterName)` identity in those state-management paths while preserving the existing name-only fallback for script opcodes that still address monsters by bare name. Verified with a successful `make -j4 engines/harvester/script.o`.

## Next Suggested Action

- Runtime-smoke monster state separation with a reused monster name: kill the CRYPT `WOLFIE`, then transition into DINING and confirm its `WOLFIE` still materializes alive with full hit points. If any scripted `SET_MONSTER` path still cross-talks across rooms, extend opcode-time monster lookup to accept room context instead of falling back to bare `monsterName`.
