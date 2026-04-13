# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, confirmed from CD3 `HARVEST.SCR` and Ghidra that native `USEITEM` dispatch ignores the parsed owner/group field and matches only carried item name plus overlapped target entity name. Annotated `UseItemRecord.owner_or_group` in Ghidra and updated ScummVM lookup so `KEWPIE_KEY` can trigger `UNLOCK_THEATRE_DOOR` on `THEATRE_DOOR` in `FOYER1` despite the script row using owner/group `FOYER`.

## Next Suggested Action

- Re-test using `KEWPIE_KEY` on `FOYER1` `THEATRE_DOOR`: confirm the door hotspot is deleted, `FOYR1_THEATRE1` is enabled, `KEWPIE_KEY` is removed from inventory, `FOYRDOOR_ANIM` plays, and the left theatre entrance changes rooms.
