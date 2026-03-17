# TRACKER

## Current Focus

- Match startup-room handoff behavior to the native `CHANGE_ROOM` / map-transition flow
- Keep room and closeup transitions scoped to behavior confirmed in Ghidra
- Only replace recursive engine behavior where the binary shows a queued pending-room handoff

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Recovered the native startup-room handoff path for `CHANGE_ROOM`, `CLOSEUP`, and `EXIT_CLOSEUP` from `parse_command_record`, `dispatch_room_event_actions`, and `run_harvester_main_loop`.
  - `CHANGE_ROOM` resolves an entrance and queues `g_pending_room_name`; `CLOSEUP` queues a raw room name and clears `g_player_present_in_room`; the main loop later consumes `g_pending_room_name` and calls `room_setup`.
  - Updated the Ghidra plate comments for those functions and changed the engine startup-room loop so `CHANGE_ROOM` now performs a same-loop handoff instead of recursing into a nested room loop and then rebuilding the source room on return.

## Next Suggested Action

1. Run the live `PCHOUSE_X4B -> PCHOUSE_2_MAP` transition and confirm the room loop no longer rebuilds `PCHOUSE` after the click.
2. Audit closeup exits against the native `EXIT_CLOSEUP` / `SAVE_GAME` handoff so closeups can eventually stop relying on recursive room-loop unwinding.
