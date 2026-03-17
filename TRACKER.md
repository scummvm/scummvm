# TRACKER

## Current Focus

- Match startup-room exit interactions to the native region-overlap and rapid-click behavior
- Keep room and closeup transitions scoped to behavior confirmed in Ghidra
- Only replace recursive or simplified engine behavior where the binary shows a distinct queued or immediate handoff

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Recovered the native startup-room handoff path for `CHANGE_ROOM`, `CLOSEUP`, and `EXIT_CLOSEUP` from `parse_command_record`, `dispatch_room_event_actions`, and `run_harvester_main_loop`.
  - `CHANGE_ROOM` resolves an entrance and queues `g_pending_room_name`; `CLOSEUP` queues a raw room name and clears `g_player_present_in_room`; the main loop later consumes `g_pending_room_name` and calls `room_setup`.
  - Updated the Ghidra plate comments for those functions and changed the engine startup-room loop so `CHANGE_ROOM` now performs a same-loop handoff instead of recursing into a nested room loop and then rebuilding the source room on return.
- Confirmed the dialogue-triggered Jimmy paper handoff was still losing `CHANGE_ROOM` semantics in engine code: `jimmy_dialogue.cpp` manually copied `nextRoomName` out of `ACTV_HOUSE_EXIT` but dropped `roomTransition`, which downgraded the queued handoff back into the legacy nested-room path.
- Confirmed another room-setup mismatch against the recovered native constructor rules: the engine was requiring room backgrounds to live under `GRAPHIC/ROOMS/*.BM`, while the documented native `spawn_object_entity_from_record` promotion rule is based on a sprite-backed object at origin whose loaded bitmap is `640x480`.
- Confirmed the remaining `PCHOUSE_2_MAP` failure was not a missing room at all: native `resolve_room_entrance` falls back from `ENTRANCE` to `MAP_ENTRANCE`, and map-entry matches open the town map UI which then resolves a final destination through `MAP_LOCATION -> destination_entrance -> ENTRANCE`.
- Confirmed two native exit-region behaviors in Ghidra before patching the room loop:
  - `check_player_region_interaction` is called from `run_harvester_main_loop` for every class `0x19` region each frame and dispatches the region action as soon as the player sprite overlaps the region bounds, the Z ranges overlap, the region is enabled, and the facing matches.
  - The mouse callback installed by `initialize_mouse_input` sets the `DAT_000d5975` fast-click latch when a left-button press happens within 20 ticks of the previous left-button release; `run_harvester_main_loop` uses that latch to jump straight into region dispatch on exit clicks except in `LAVAPIT`, `RMNBATH`, and `BOWLSNTRY1`.
- Updated the engine room loop to match those confirmed behaviors:
  - walking into an enabled exit region now triggers the region action without requiring that exact hotspot to have been clicked first;
  - a native-style rapid second left click on an exit region now dispatches immediately instead of queuing a walk-up first;
  - region activation now stops active room movement before dispatch so exit/dialogue actions do not continue the previous walk path.

## Next Suggested Action

1. Re-test `PCHOUSE_X4B` in two ways: walk across the exit after clicking nearby ground, and rapid-double-click the exit itself. Both should now enter the town-map path without waiting for the player to finish a queued walk.
2. Re-test a non-map exit plus one of the native fast-click blacklist rooms (`LAVAPIT`, `RMNBATH`, or `BOWLSNTRY1`) to confirm the new shortcut is limited to the same rooms the binary allows.
