# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, confirmed `HARVEST.SCR` room `FLESH` enters through `D2_2_F1`, enables `TALK_HERPOTT_T`, and issues `START_DIALOG "POTTS_FLESH"` when `PC_TALKED_TO_POTTS` is false. Confirmed in Ghidra that native dialogue dispatch table entry `POTTS_FLESH` points to `handle_talk_to_mr_potts` like the `MR_POTTS` variants, and patched `MrPottsDialogueHandler::matchesNpc()` to accept `POTTS_FLESH`.

## Next Suggested Action

- Re-test entering `FLESH` from `D2_2_F1` with `HARVEST_BLADE` in inventory and verify the `POTTS_FLESH` dialogue starts automatically, `PC_TALKED_TO_POTTS` is set, `MONSTERFY_POTTS` runs after the dialogue branch, and `POTTS_FLESH` can also be clicked/used as a talkable room NPC.
