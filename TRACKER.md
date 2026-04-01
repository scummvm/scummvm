# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, aligned the Harvester close-up unwind path with the original binary by preserving the parent room save snapshot before nested close-ups and reapplying that parent state on `EXIT_CLOSEUP`, instead of restarting from the nested close-up save state. Verified with a successful `make -j4 engines/harvester/script.o engines/harvester/dialogue.o engines/harvester/flow.o engines/harvester/room.o`. Also decoded CD3 `HARVEST.SCR` and confirmed the BAR strong-beer chain: `DEATV_BAR_DOORS` disables `BAR_DINING`/`BAR_FOUNT`, sets `BARANIM_FLAG`, and exits the close-up; on BAR re-entry, room on-enter tag `CHECK_MONST` checks `BARANIM_FLAG`, arms `BAR_MONSTZ_T` and `BAR_TIMER`, starts `EVIL_PC_ANIM` plus `magic2.wav` after 3 ticks, and spawns monster `EVILPC` after 8 ticks.

## Next Suggested Action

- Do a runtime smoke test for the CD3 strong-beer path: enter `BAR`, open `BARCU`, operate `BARCUXXBEER`, confirm the close-up exits back to `BAR`, BAR room-entry runs `CHECK_MONST`, `BAR_MONSTZ_T` starts `EVIL_PC_ANIM` with `magic2.wav`, and `BAR_TIMER` then spawns `EVILPC`.
