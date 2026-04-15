# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 15, 2026, confirmed from `HARVEST.SCR` that `SUPLYDOOR_ANIM` is explicitly started by the `OPEN_SUP_DOOR1` use-item chain after `SUPLY2_BOILKEY` is used on `SUPPLY1_DOOR`. Adjusted room animation runtime handling so non-looping visible room animations do not auto-advance on room entry before a `SET_ANIM` command starts them.

## Next Suggested Action

- Re-test the supply-room boiler path: enter `SUPPLY1` from `SUPPLY2_2_SUPPLY1` before using `SUPLY2_BOILKEY` and verify `SUPLYDOOR_ANIM` remains on its first frame, then use the boiler key on `SUPPLY1_DOOR` and verify the door animation starts only from the `OPEN_SUP_DOOR1` command chain.
