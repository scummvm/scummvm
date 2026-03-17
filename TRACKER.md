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

- Ported `handle_talk_to_mcknight @ 0x2c770` into a dedicated `engines/harvester/npc/mcknight_dialogue.*` handler.
  - Native McKnight dialogue is another compact visible handler: `LOOK_SAFE_2ND` forces `0x15d0`, `LOOK_SAFE_1ST` reads an additional unresolved gate before `0x15c9`, the no-item path is a one-shot intro (`0x1587`) that falls through to `0x15e4`, and the item path handles the shared Whaley/casket/ledger/note evidence cluster.
  - The engine now routes `MCKNIGHT` through a concrete handler with matching talk-state and evidence writes; the extra `LOOK_SAFE_1ST` gate remains locally represented until its gameplay-side setter is recovered.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidates are `LOOMIS`, `MOYNAHAN`, `MR_POTTS`, and `MRS_POTTS`, followed by the larger hidden-topic handlers such as Boyle.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
