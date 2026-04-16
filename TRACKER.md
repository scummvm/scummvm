# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `923`
- Named/non-`FUN_*`: `864`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 16, 2026, fixed the Loomis dirty-magazine dialogue branch so its queued runtime mutation is also marked as a visual room-state change. This lets the current room refresh remove the hidden `LOOMIS` actor and update the inventory/drawer visuals after `GRAPHIC/FST/C048.FST`. Verified `engines/harvester/npc/loomis_dialogue.o` builds.

## Next Suggested Action

- Runtime-test giving `INV_MAG` to `LOOMIS` in `SHRFOFC`: after selecting `Sure!` and playing `GRAPHIC/FST/C048.FST`, confirm Loomis is no longer visible or clickable, the dirty magazine is gone from inventory, and the sheriff drawer visuals are refreshed.
