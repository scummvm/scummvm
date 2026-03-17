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

- Ported `handle_talk_to_madam @ 0x2adc0` into a dedicated `engines/harvester/npc/madam_dialogue.*` handler.
  - Native Madam dialogue is a single interrupt branch: when `MADAM_INTERRUPT_CONVERSATION` is set, native clears the flag and plays `0x1537` for `MADAM` with head variant `1`; otherwise the handler returns without dialogue.
  - The engine now routes `MADAM` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Port `handle_talk_to_gladiator @ 0x2c630`, which shares the same interrupt-flag shape.
   - Ghidra decompilation shows `GLADIATOR_INTERRUPT_CONVERSATION`, a clear-to-zero write, and a single `0x69c` `GLADIATOR` line with head variant `1`.
2. After Gladiator, resume the remaining low-complexity visible handlers before returning to the larger hidden-topic ports.
   - The next best candidates are the short non-hidden handlers that do not require new shared state reconstruction.
