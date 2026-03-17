# TRACKER

## Current Focus

- Implement the remaining Ghidra-identified NPC dialogue handlers in the engine
- Keep each NPC port scoped to confirmed native behavior only
- Update `TRACKER.md` and commit after each completed NPC

## Progress

- Program: `HARVEST.LE`
- Total functions: `886`
- Named/documented: `569`
- Still `FUN_*` / undocumented: `317`

## Last Confirmed Action

- Ported `handle_talk_to_fireman1 @ 0x33850` into a dedicated `engines/harvester/npc/fireman1_dialogue.*` handler.
  - Native Fireman 1 dialogue is a two-outcome local talk-state branch: the first talk clears `g_fireman1_talk_state_block` and plays `0x453` for `FIREMAN1`, while subsequent talks fall through to `0x463`.
  - The engine now routes `FIREMAN1` through a concrete handler with matching per-NPC state instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_fireman2 @ 0x33940` as the next short local-state handler.
   - Ghidra decompilation currently shows the same two-outcome shape as Fireman 1: the first talk clears `g_fireman2_talk_state_block` and plays `0x5b9`, while the steady path plays `0x5c7`.
2. After the short visible handlers are exhausted, return to the bounded multi-branch ports starting with Boyle.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
