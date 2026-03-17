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

- Ported `handle_talk_to_cloak_atnd @ 0x2db80` into a dedicated `engines/harvester/npc/cloak_atnd_dialogue.*` handler.
  - Native Cloak Attendant dialogue is a small no-item/item split: if no item is presented and `CLEANED_CLOTHES` is still clear, it plays `0x1066` for `CLOAK_ATND`; if `BARCASHFIVE` is presented, it dispatches `CLEAN_CLOTHES`.
  - The engine now routes `CLOAK_ATND` through a concrete handler that mirrors the same flag check and action-tag dispatch, including queued interaction side effects.

## Next Suggested Action

1. Resume the remaining low-complexity visible handlers before returning to the bounded multi-branch ports.
   - The remaining short visible handler before the evidence-heavy pass is `DARK_WOMAN`.
2. After that short-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - After `DARK_WOMAN`, the next best bounded visible ports are the evidence-heavy but still linear handlers such as `BUSTER`, `BUTCHER`, `PHELPS`, `SWELL`, and `PARSONS`, before returning to Boyle.
