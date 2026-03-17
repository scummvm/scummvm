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

- Ported `handle_talk_to_librarian @ 0x28f40` into a dedicated `engines/harvester/npc/librarian_dialogue.*` handler.
  - Native Librarian dialogue is a small inventory gate: if the player already has `CLUE` or `CAINBOOK`, or explicitly presents `CAINBOOK`, she grants `CLUE` to inventory and plays `0x1db1`; otherwise she falls through to the default `0x1d5b` response.
  - The engine now routes `LIBRARIAN` through a concrete handler with the same inventory check and `CLUE` handoff behavior.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next bounded cleanup candidates before the largest trees are `NUDE_MAN`, `MEMB_DIR`, and `KARIN`, followed by `LOOMIS`, `MOYNAHAN`, `MR_POTTS`, and `MRS_POTTS`.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
