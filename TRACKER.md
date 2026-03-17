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

- Ported `handle_talk_to_beggar @ 0x34d40` into a dedicated `engines/harvester/npc/beggar_dialogue.*` handler.
  - Native Beggar dialogue is a single interrupt branch: when `BEGGAR_INTERRUPT_CONVERSATION` is set, native dispatches the `BEGGAR_DIALOG_3` action tag, then plays `0x1276` for `BEGGAR` with head variant `1`; otherwise the handler returns without dialogue.
  - The engine now routes `BEGGAR` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_madam @ 0x2adc0` as the next small interrupt-driven NPC handler.
   - Ghidra decompilation currently shows a single interrupt flag gate: when `MADAM_INTERRUPT_CONVERSATION` is set, native clears it and plays `0x1537` for `MADAM` with head variant `1`; otherwise the handler returns without dialogue.
2. After Madam, port `handle_talk_to_gladiator @ 0x2c630`, which shares the same interrupt-flag shape.
   - Gladiator is the same pattern with `GLADIATOR_INTERRUPT_CONVERSATION`, a clear-to-zero write, and a single `0x69c` `GLADIATOR` line.
