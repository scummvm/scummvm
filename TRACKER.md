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

- Ported `handle_talk_to_maint_man @ 0x2b6a0` into a dedicated `engines/harvester/npc/maint_man_dialogue.*` handler.
  - Native Maintenance Man dialogue is a compact flag-driven selector with no local room state: `MAINTENANCE_MAN_FIRST_CONVERSATION` forces `0xdaf`, `MAINTENANCE_MAN_THIRD_CONVERSATION` forces `0xdbc`, `MAINTENANCE_MAN_FOURTH_CONVERSATION` forces `0xdc4`, and the default path falls through to `0xdb6`.
  - The engine now routes `MAINT_MAN` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_beggar @ 0x34d40` as the next small interrupt-driven NPC handler.
   - Ghidra decompilation currently shows one flag gate plus one action tag: `BEGGAR_INTERRUPT_CONVERSATION` triggers `dispatch_action_tag_if_set("BEGGAR_DIALOG_3", "BEGGAR")` and then plays `0x1276`; otherwise the native handler returns without dialogue.
2. Add the minimal runtime callback needed for the interrupt/action-tag handlers, then continue through Beggar, Madam, and Gladiator.
   - Those three handlers are the next confirmed low-complexity group after the visible flag-driven ports and they share the same native shape.
