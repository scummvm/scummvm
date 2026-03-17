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

- Ported `handle_talk_to_buster @ 0x2b7c0` into a dedicated `engines/harvester/npc/buster_dialogue.*` handler.
  - Native Buster dialogue is a straight evidence/no-item handler: the no-item path walks three one-shot intro lines (`0x163c`, `0x1828`, `0x18c5`) before settling on `0x1868`, while the item path handles Whaley/Herrill photo, casket-photo evidence, and note/checkbook evidence with the same shared `SHOWN_*` writes used elsewhere.
  - The engine now routes `BUSTER` through a concrete handler with matching local talk-state and shared evidence-flag writes.

## Next Suggested Action

1. Continue the evidence-heavy but still linear visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are `BUTCHER`, `PHELPS`, `SWELL`, `PARSONS`, and `WHALEY`, which mostly reuse the already-confirmed shared evidence state writes and straight no-item talk-state branches.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
