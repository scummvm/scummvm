# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, confirmed `HARVEST.SCR` room `NARCISSISM` spawns NPC `DARK_WOMAN` with monsterfy target `DARK_WOMAN_MONSTER`, and confirmed in Ghidra that `handle_talk_to_dark_woman` plays insult response line `0x4ce2` then calls `queue_live_named_npc_death_or_monsterfy_transition("DARK_WOMAN")`. Updated the Dark Woman insult branches to queue the live NPC monsterfy transition and mark the current room visual state dirty.

## Next Suggested Action

- Re-test entering `NARCISSISM` from `C1_2_G2`, choose "Uh, you're not that hot...", and verify `DARK_WOMAN` begins the monsterfy animation, `DARK_WOMAN_MONSTER` appears active afterward, and the `SET_D_W_FLAG` death command opens exits `G_1` and `G_3` after the monster is defeated.
