# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, verified two live-room combat behaviors in Ghidra before changing the Harvester engine. Native `queue_named_npc_death_or_monsterfy_transition` only queues desired state `0x35` for a live class-4 NPC and leaves record suppression until `update_actor_runtime_state` reaches the death-bank terminal frame, while `update_player_combat_avatar_state` and `update_actor_runtime_state` drive room-combat vertical walking with an independent signed `z_velocity_step` instead of inferring depth direction from screen-Y remapping. Updated the ScummVM room loop and script path to defer live `KILL_NPC` / `MONSTERFY` transitions until the in-room death bank completes, preserve the live NPC entity across the intermediate runtime-state refresh, and switch keyboard vertical combat movement to the native signed depth step.

## Next Suggested Action

- In-engine, replay the Chessmaster combat branch from `ADD_CHESS_KING_MONSTER` through `CHESSKING_DCOM2` and confirm three outcomes together: CHESSKING remains targetable once the player walks onto his depth plane, the Chessmaster dies through his in-room corpse bank instead of disappearing immediately, and the post-death `ADD_CHESS_CU` interaction becomes available so the key can be collected. If any step still diverges, capture the exact action tag and frame/state transition from the debug log for another Ghidra comparison.
