# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 11, 2026, verified native `MONSTERFY` handling in Ghidra. `dispatch_room_event_actions` case `0x0e` queues live NPC actor state `0x35` through `queue_npc_death_or_monsterfy_transition` without requiring a damage argument. Updated ScummVM's deferred live NPC death/monsterfy queue so blank scripted damage arguments still queue the transition using the bludge animation selector.

## Next Suggested Action

- In-engine, trigger the `BALLROM1` maintenance man fourth warning and confirm that after `"All right, buddy... you axed for it!"`, the `MONSTERFY MAINT_MAN` action starts the NPC death/monsterfy animation and spawns the configured monster target.
