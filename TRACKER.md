# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, confirmed in Ghidra that `dispatch_room_event_actions` applies `SET_FLAG` without rebuilding the room or rereading the palette; `SHOW_TEXT` draws over the existing room palette, `CHANGE_LIGHTING NONE` clears to black, and `GOFLIC` starts the FST without an intermediate same-room `room_setup`. Updated the engine to refresh the live room only for current-room visual mutations, not flag-only or off-room runtime mutations.

## Next Suggested Action

- Re-run the DNAEXT night-5 fire exit path with `--debugflags=room,scene,general` and confirm the `SET_FIRE_TEXT1` modal remains on the night/dim palette until `CHANGE_LIGHTING NONE` blacks out before `GRAPHIC/FST/OVERNITE.FST`.
