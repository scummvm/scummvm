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

- Ported `handle_talk_to_vet @ 0x2fd20` into a dedicated `engines/harvester/npc/vet_dialogue.*` handler.
  - Native Vet dialogue is a one-shot interrupt branch on `VET_INTERRUPT`: when the flag is set it is cleared and `0x6d9` plays for `VET`, otherwise the handler falls through.
  - The engine now routes `VET` through a concrete handler that mirrors the same flag clear and conditional line playback.

## Next Suggested Action

1. Resume the remaining low-complexity visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are the remaining short non-hidden handlers that do not require new shared state reconstruction: `CLOAK_ATND` and `DARK_WOMAN`.
2. After that short-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
