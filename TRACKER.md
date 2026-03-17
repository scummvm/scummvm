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

- Ported `handle_talk_to_phelps @ 0x29920` into a dedicated `engines/harvester/npc/phelps_dialogue.*` handler.
  - Native Phelps dialogue is another linear evidence/no-item handler: the no-item path is a one-shot intro (`0x1b44`) that falls through to `0x1bf4`, while the item path handles the shared Whaley/casket/ledger/note evidence cluster plus the special `QUARTER` and `REWARD_MONEY` action-tag branches.
  - The engine now routes `PHELPS` through a concrete handler with matching local talk-state, shared evidence-flag writes, and action-tag side effects.

## Next Suggested Action

1. Continue the evidence-heavy but still linear visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are `SWELL`, `PARSONS`, `WHALEY`, and `MCKNIGHT`, which mostly reuse the already-confirmed shared evidence state writes and straight no-item talk-state branches.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
