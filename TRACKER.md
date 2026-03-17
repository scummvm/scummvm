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

- Ported `handle_talk_to_valet @ 0x2eda0` into a dedicated `engines/harvester/npc/valet_dialogue.*` handler.
  - Native Valet dialogue is a fixed single-line handler that always plays `0xf2c` for `VALET`; there is no topic tree, item branch, or local state in the recovered function.
  - The engine now routes `VALET` through a concrete handler that mirrors the same unconditional line playback.

## Next Suggested Action

1. Resume the remaining low-complexity visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are the remaining short non-hidden handlers that do not require new shared state reconstruction: `VET`, `CLOAK_ATND`, and `DARK_WOMAN`.
2. After that short-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
