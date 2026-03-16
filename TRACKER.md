# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `887`
- Named/documented: `565`
- Still `FUN_*` / undocumented: `322`

## Last Confirmed Action

- Recovered `SET_REGION` and `START_DIALOG` from `dispatch_room_event_actions @ 0x60ee0`, then aligned the startup script/runtime flow with the native continuation behavior.
  - Confirmed `case 0x16` finds a `RegionRecord` by name, treats `arg2 == "F"` as the disable sentinel, updates `start_enabled`, and toggles the live room-region entity from that runtime state; ScummVM now keeps runtime region records and materializes room regions from them instead of the immutable parse-time table.
  - Confirmed `case 0x1a` refreshes/fades the room palette if needed, calls `run_npc_dialogue(arg1, 0)`, then resumes at `arg4` unless the native post-dialogue abort flag trips; ScummVM now suspends `START_DIALOG`, runs the named NPC dialogue with no presented item, drains queued dialogue-side effects, and resumes the continuation tag afterward.
  - Unified direct room-NPC clicks and scripted `START_DIALOG` through the same post-dialogue interaction path, so no-item dialogue handlers can now mutate startup state consistently.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the dispatcher/room-loop changes.

## Next Suggested Action

1. Recover the native post-dialogue abort flag checked immediately after `run_npc_dialogue` in `dispatch_room_event_actions @ 0x60ee0`.
   - ScummVM now skips `START_DIALOG` continuations when the dialogue result already transitions rooms, but the DOS handler also has a separate `DAT_000d60bc` early-return path that still needs a confirmed engine-side mapping.
2. Recover the monster-record replacement half of `MONSTERFY`.
   - `DIAL_JIM_4_D -> MONST_JIM` is now reachable through the restored `START_DIALOG` continuation, but ScummVM still only sets the NPC death/monsterfy flag and suppresses the original NPC; the native monster-record spawn/replacement path is still missing.
