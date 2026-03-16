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

- Recovered the native post-dialogue abort flag `DAT_000d60bc` around `dispatch_room_event_actions @ 0x60ee0`, then aligned the startup dialogue continuation flow with the confirmed nested-dispatch behavior.
  - Confirmed `dispatch_room_event_actions` `case 0x1a` calls `run_npc_dialogue(arg1, 0)`, waits for mouse release, then returns early when `DAT_000d60bc != 0` instead of always resuming `arg4`.
  - Confirmed `run_inventory_screen @ 0x7ec02`, `handle_target_interaction @ 0x801cc`, and three `update_actor_runtime_state` paths (`0x50a50`, `0x514fb`, `0x5154e`) all set `DAT_000d60bc = 1` immediately after nested `dispatch_room_event_actions` calls, establishing it as the native “abort the caller’s remaining command chain” signal.
  - Confirmed `run_game_over_screen @ 0x7c749` stores `1` or `2`, and `run_harvester_main_loop` clears `1` to restart the loop but treats `2` as the outer exit path; ScummVM only needed the value-`1` nested-abort behavior for current startup dialogue parity.
  - ScummVM now propagates a dedicated nested action-tag abort bit through queued dialogue interactions, so `START_DIALOG` continuations are skipped not only on room transitions but also when the dialogue triggers a nested startup action-tag analogue such as Jimmy’s `ACTV_HOUSE_EXIT`.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the abort-path runtime changes.

## Next Suggested Action

1. Recover the monster-record replacement half of `MONSTERFY`.
   - `DIAL_JIM_4_D -> MONST_JIM` is now reachable through the restored `START_DIALOG` continuation, but ScummVM still only sets the NPC death/monsterfy flag and suppresses the original NPC; the native monster-record spawn/replacement path is still missing.
2. Confirm whether any startup dialogue/action-tag path needs a value-`2` / outer-exit analogue of `DAT_000d60bc`.
   - Current startup parity only needs the confirmed value-`1` nested-abort case; the DOS main loop also treats `2` as a high-level exit request, but no startup-script path has been tied to that behavior yet.
