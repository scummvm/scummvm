# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Re-ran the native room-NPC spawn/update path from `room_setup` through `spawn_npc_entity_from_record` and `update_actor_runtime_state`.
  - Native room NPCs are not allowed to free-run their full ABM. Ordinary passive NPC spawn seeds actor state `0x34` and clamps the live ambient frame window to `0..0x3b`; the higher banks are reserved for later state-machine-driven reactions, deaths, and monsterfy transitions.
  - The engine startup-room NPC path now matches that bounded ambient loop instead of cycling every decoded frame in the ABM.

## Next Suggested Action

1. Exercise several room NPC ABMs with known post-`0x3b` reaction/death banks to confirm the bounded ambient loop fixes the visible over-seek without breaking passive idle playback.
2. Continue the native actor-system pass on room monsters and any NPCs that rely on leaving ambient state `0x34`, so later startup-room behavior can move from static frame-window emulation toward the real class-4/class-6 state machine where needed.
