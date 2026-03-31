# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, patched Harvester’s class-`4` NPC death finalization so a completed NPC kill now clears the authoritative runtime NPC record (`active=0`, `visible=0`, `savedVisible=0`, `deathOrMonsterfy=1`) and immediately removes the live scene actor before the `onDeathActionTag` refresh path runs. This closes the gap where BIG_EYE could finish its death bank yet remain eligible for redisplay during the post-death room update. Verified with `make -j4 scummvm`.

## Next Suggested Action

- Re-run the in-engine `EYEHALL` repro and confirm the BIG_EYE death bank ends with the actor removed, `EYEDOOR` still enables `EYE_MAIN`, and no other class-`4` NPC deaths regress by disappearing too early or failing monsterfy transitions.
