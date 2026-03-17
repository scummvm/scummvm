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

- Ported `handle_talk_to_authority @ 0x2b730` into a dedicated `engines/harvester/npc/authority_dialogue.*` handler.
  - Native Authority dialogue is a small flag-driven selector with no local room state: `STOP_AUTHOR_TALK` suppresses dialogue entirely, `IF_DON_T_EAT_THE_FOOD` forces `0x3992`, and otherwise the handler picks `0x398a` or `0x399b` based on `IF_YOU_EAT_THE_FOOD_AND_DEFEAT_THE_ENEMIES`.
  - The engine now routes `AUTHORITY` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_maint_man @ 0x2b6a0` as the next small flag-driven NPC handler.
   - Ghidra decompilation currently shows a compact four-way branch on the maintenance-man conversation flags, with `0xdaf`, `0xdb6`, `0xdbc`, and `0xdc4` as the only native outcomes.
2. After the remaining single-branch handlers are covered, resume the larger hidden-topic ports starting with Boyle.
   - Boyle already has bounded hidden keyword and gas-can branches, so he remains the highest-signal multi-branch NPC once the trivial handlers are out of the way.
