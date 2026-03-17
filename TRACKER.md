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

- Ported `handle_talk_to_ryder @ 0x38c80` into a dedicated `engines/harvester/npc/ryder_dialogue.*` handler.
  - Native Ryder dialogue is a compact stateful visible handler: `BURNED_TV_STATION` switches him outright to the `RYDER_TV_OUT` bark at `0x5a6`, otherwise the no-item path is a one-shot intro (`0x477`) that falls through to `0x4cf`, and the item path handles Whaley/casket/note evidence only.
  - The engine now routes both `RYDER` and `RYDER_TV_OUT` through a concrete handler with matching speaker selection, local talk-state, and shared evidence-flag writes.

## Next Suggested Action

1. Continue the evidence-heavy but still linear visible handlers before returning to the bounded multi-branch ports.
   - The next best candidate in the visible linear bucket is `MCKNIGHT`; after that, the remaining work shifts back toward more stateful or hidden-topic handlers like `LOOMIS`, `MOYNAHAN`, `MR_POTTS`, and Boyle.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
