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

- Ported `handle_talk_to_inquisitor @ 0x2af70` into a dedicated `engines/harvester/npc/inquisitor_dialogue.*` handler.
  - Native Inquisitor dialogue is a one-shot local talk-state branch gated by the global `PAIN_SOLVED` flag: when the puzzle is still unsolved and `g_inquisitor_talk_state_block` is set, native clears it and plays `0x18e7` for `INQUISITOR`.
  - The engine now routes `INQUISITOR` through a concrete handler with matching per-NPC state instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_authority @ 0x2b730` as the next small flag-driven NPC handler.
   - Ghidra decompilation currently shows three flag-controlled outcomes: `STOP_AUTHOR_TALK` suppresses dialogue entirely, `IF_DON_T_EAT_THE_FOOD` forces `0x3992`, and otherwise the handler picks `0x398a` or `0x399b` based on `IF_YOU_EAT_THE_FOOD_AND_DEFEAT_THE_ENEMIES`.
2. After the remaining single-branch handlers are covered, resume the larger hidden-topic ports starting with Boyle.
   - Boyle already has bounded hidden keyword and gas-can branches, so he remains the highest-signal multi-branch NPC once the trivial handlers are out of the way.
