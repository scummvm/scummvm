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

- Ported `handle_talk_to_chessmaster @ 0x2d900` into a dedicated `engines/harvester/npc/chessmaster_dialogue.*` handler.
  - Native Chessmaster dialogue is a fixed single-line bark: `play_dialogue_line(0x4115, "CHESSMASTER")`, with no item gating, no room-local state, and no shared dialogue-state writes.
  - The engine now routes `CHESSMASTER` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_mother @ 0x2b5b0` as the next small state-gated NPC handler.
   - Ghidra decompilation currently shows a single local talk-state bit gate: when `g_mother_talk_state_block` is set, native clears it and plays `0x26d6` for `MOTHER`.
2. After the remaining single-branch handlers are covered, resume the larger hidden-topic ports starting with Boyle.
   - Boyle already has bounded hidden keyword and gas-can branches, so he remains the highest-signal multi-branch NPC once the trivial handlers are out of the way.
