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

- Ported `handle_talk_to_priest @ 0x29730` into a dedicated `engines/harvester/npc/priest_dialogue.*` handler.
  - Native Priest dialogue is a one-shot gate on `PC_TALKED_TO_PRIEST`: the first conversation sets the flag and plays `0xcb2` for `PRIEST`, and later talks do nothing.
  - The engine now routes `PRIEST` through a concrete handler that mirrors the same flag write and single-line behavior.

## Next Suggested Action

1. Resume the remaining low-complexity visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are the remaining short non-hidden handlers that do not require new shared state reconstruction: `VALET`, `VET`, `CLOAK_ATND`, and `DARK_WOMAN`.
2. After that short-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
