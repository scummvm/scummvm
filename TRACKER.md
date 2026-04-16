# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, checked Inquisitor, Gladiator, and Beggar monsterfy dialogue paths against `HARVEST.SCR` and Ghidra. Confirmed `handle_talk_to_gladiator` calls `queue_live_named_npc_death_or_monsterfy_transition("GLADIATOR")` after hostile branches, and `handle_talk_to_beggar` calls `queue_live_named_npc_death_or_monsterfy_transition("BEGGAR")` at dialogue completion. Updated both handlers to queue the live monsterfy transition and mark current room visual state dirty. Confirmed Inquisitor dialogue only dispatches `START_INQ_TIM`; its later `MONSTERFY "INQUISITOR"` remains script/timer-driven.

## Next Suggested Action

- Re-test `MERCY` hostile Gladiator branches and `CHARITY` Beggar dialogue completion, and verify each live NPC begins the monsterfy animation before its monster record becomes active. Also re-test `PAIN` long enough for `INQUIST_ATTACK_TIMER` to fire and verify Inquisitor still monsterfies through the script timer path.
