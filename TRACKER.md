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

- Ported `handle_talk_to_curator @ 0x34ca0` into a dedicated `engines/harvester/npc/curator_dialogue.*` handler.
  - Native Curator dialogue is a fixed single-line bark: `play_dialogue_line(0x425, "CURATOR")`, with no item gating, no room-local state, and no shared dialogue-state writes.
  - The engine now routes `CURATOR` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_lodge_chef @ 0x38170` as the next one-line NPC handler.
   - Ghidra decompilation currently shows a fixed `play_dialogue_line(0x1b10, "LODGE_CHEF")` body with no visible state, making it the next lowest-risk completion.
2. After the remaining one-line handlers are covered, resume the larger hidden-topic ports starting with Boyle.
   - Boyle already has bounded hidden keyword and gas-can branches, so he remains the highest-signal multi-branch NPC once the trivial handlers are out of the way.
