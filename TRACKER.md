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

- Ported `handle_talk_to_mother @ 0x2b5b0` into a dedicated `engines/harvester/npc/mother_dialogue.*` handler.
  - Native Mother dialogue is a one-shot local talk-state branch: when `g_mother_talk_state_block` is set, native clears it and plays `0x26d6` for `MOTHER`; otherwise the handler returns without dialogue.
  - The engine now routes `MOTHER` through a concrete handler with matching per-NPC state instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_inquisitor @ 0x2af70` as the next small state-gated NPC handler.
   - Ghidra decompilation currently shows a single local talk-state gate plus the `PAIN_SOLVED` flag: when the puzzle is not solved and `g_inquisitor_talk_state_block` is set, native clears it and plays `0x18e7` for `INQUISITOR`.
2. After the remaining single-branch handlers are covered, resume the larger hidden-topic ports starting with Boyle.
   - Boyle already has bounded hidden keyword and gas-can branches, so he remains the highest-signal multi-branch NPC once the trivial handlers are out of the way.
