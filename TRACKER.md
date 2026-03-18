# TRACKER

## Current Focus

- Recover the native ESC room-menu systems one item at a time from `run_main_menu`
- Keep each menu item implementation grounded in confirmed Ghidra behavior before broadening engine UI helpers
- Extend engine-side persistence only where the native menu paths actually require it

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Recovered and implemented the native `SAVE GAME` room-menu path from `run_save_game_menu` and `sync_current_room_runtime_state_before_save_load`.
  - Confirmed the save screen uses `SAVEGAME.BM` + `SAVEGAME.PAL`, exposes 25 fixed slots, and routes slot-click, `Enter`, and the lower-left action area through the same inline text-entry-and-save path.
  - Confirmed the native pre-save sync step copies live room state back into persistent records before serialization, and that the room menu cancels via `ESC` or the lower-right action area.
- Implemented the engine-side `SAVE GAME` screen on the ESC room menu:
  - room-menu `SAVE GAME` now opens a native-shaped 25-slot save UI with inline text entry and immediate commit after confirmation;
  - Harvester now has a real ScummVM save serializer for the current startup/runtime subset, including room resume context, runtime script records, and Steve's current HP;
  - launcher/runtime loads now resume via `materializeRoomState()` so a loaded room comes back without replaying room-enter command chains;
  - the Harvester metaengine now exposes the same `0..24` slot range the native menu uses and disables generic autosave slot reservation.

## Next Suggested Action

1. Recover the native `QUIT GAME` confirmation branch from `run_main_menu`, especially the `textbox4.bm` prompt, YES/NO hit regions, keyboard shortcuts, and the post-confirm quit handoff.
2. Mirror that confirmation flow in the ESC room menu so `QUIT GAME` stops being a stub without regressing palette restoration or room-loop cleanup on cancel.
