# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `929`
- Named/documented: `825`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, checked native class-4 NPC death handling in `HARVEST.LE` against Ghidra. Confirmed `update_actor_runtime_state` keeps a completed non-monsterfied NPC death as current-room actor state `0x38` while setting `NpcRecord.death_or_monsterfy_flag` for later room setup suppression. Updated ScummVM room combat handling so MERCY old man/old woman death sprites remain in the current room after their on-death script refresh.

## Next Suggested Action

- Re-test `MERCY` from entrance `K1_2_I2`. Kill `OLDMAN` and `OLDWOMAN`, verify both death sprites remain visible after `OLD_MAN_COM` / `OLD_WOMAN_COM` actions refresh the room, and verify leaving/re-entering still suppresses the killed NPCs.
