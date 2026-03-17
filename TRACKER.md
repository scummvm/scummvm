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

- Ported `handle_talk_to_whaley @ 0x23ec0` into a dedicated `engines/harvester/npc/whaley_dialogue.*` handler.
  - Native Whaley dialogue is another linear evidence/no-item handler with one pre-empting plot bark: `BUSTED_SCREWING_MIDGAME` forces `0x143b`, the no-item path is a one-shot intro (`0x12b6`) that falls through to `0x1385`, and the item path handles the shared casket/ledger/note evidence cluster while leaving Whaley’s own photo branch as a plain `0x1411` reply.
  - The engine now routes `WHALEY` through a concrete handler with matching local talk-state and shared evidence-flag writes only where the native handler actually performs them.

## Next Suggested Action

1. Continue the evidence-heavy but still linear visible handlers before returning to the bounded multi-branch ports.
   - The next best candidate in the visible linear bucket is `MCKNIGHT`; after that, the remaining work shifts back toward more stateful or hidden-topic handlers.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
