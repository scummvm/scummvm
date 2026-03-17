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

- Ported `handle_talk_to_fireman2 @ 0x33940` into a dedicated `engines/harvester/npc/fireman2_dialogue.*` handler.
  - Native Fireman 2 dialogue is a two-outcome local talk-state branch: the first talk clears `g_fireman2_talk_state_block` and plays `0x5b9` for `FIREMAN2`, while subsequent talks fall through to `0x5c7`.
  - The engine now routes `FIREMAN2` through a concrete handler with matching per-NPC state instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Resume the remaining low-complexity visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are the short non-hidden handlers that do not require new shared state reconstruction, followed by the already-bounded multi-branch Boyle port.
2. After that short-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
