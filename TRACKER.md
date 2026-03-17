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

- Ported `handle_talk_to_parsons @ 0x397c0` into a dedicated `engines/harvester/npc/parsons_dialogue.*` handler.
  - Native Parsons dialogue is another linear evidence/no-item handler: the no-item path is a one-shot intro (`0xdcc`) that falls through to `0xe65`, while the item path handles the shared Whaley/casket/ledger/note evidence cluster plus the TV deed branch.
  - The engine now routes `PARSONS` through a concrete handler with matching local talk-state and shared evidence-flag writes.

## Next Suggested Action

1. Continue the evidence-heavy but still linear visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are `WHALEY` and `MCKNIGHT`, which mostly reuse the already-confirmed shared evidence state writes and straight no-item talk-state branches.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
