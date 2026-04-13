# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, verified native `ADD` / `DELETE` object visibility behavior in `HARVEST.LE` via Ghidra (`dispatch_room_event_actions`, `set_object_visibility_for_owner_or_room`, and `room_setup`) and patched ScummVM room materialization so hidden/deleted room objects remain in script runtime state but do not re-enter `RoomSetupState` or the live scene until an `ADD` makes them visible again.

## Next Suggested Action

- Re-run the `EYEHALL` combat case, kill `BIG_EYE`, and confirm `EYEDOOR2 DELETE EYEHALL BIGEYE` leaves `BIGEYE` out of the materialized room object list and scene after the runtime refresh while `EYE_MAIN` remains enabled.
