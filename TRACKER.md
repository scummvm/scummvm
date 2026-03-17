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

- Ported `handle_talk_to_dark_woman @ 0x33610` into a dedicated `engines/harvester/npc/dark_woman_dialogue.*` handler.
  - Native Dark Woman dialogue is a short two-branch handler: presenting `HANDMIRROR` plays `0x4d03` for `DARK_WOMAN`, otherwise the one-shot `g_dark_woman_talk_state_block` branch clears itself and plays `0x4cd5`.
  - The engine now routes `DARK_WOMAN` through a concrete handler with matching item handling and a local one-shot talk-state bit.

## Next Suggested Action

1. Continue the evidence-heavy but still linear visible handlers before returning to the bounded multi-branch ports.
   - The next best candidates are `BUSTER`, `BUTCHER`, `PHELPS`, `SWELL`, and `PARSONS`, which mostly reuse the already-confirmed shared evidence state writes and straight no-item talk-state branches.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
