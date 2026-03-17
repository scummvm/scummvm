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

- Ported `handle_talk_to_gladiator @ 0x2c630` into a dedicated `engines/harvester/npc/gladiator_dialogue.*` handler.
  - Native Gladiator dialogue is a single interrupt branch: when `GLADIATOR_INTERRUPT_CONVERSATION` is set, native clears the flag and plays `0x69c` for `GLADIATOR` with head variant `1`; otherwise the handler returns without dialogue.
  - The engine now routes `GLADIATOR` through a concrete handler instead of falling back to the unsupported NPC path.

## Next Suggested Action

1. Resume the remaining low-complexity visible handlers before returning to the larger hidden-topic ports.
   - The next best candidates are the short non-hidden handlers that do not require new shared state reconstruction, starting with the still-unported simple bark/flag handlers such as `FIREMAN1`, `FIREMAN2`, and comparable short natives.
2. After that short-handler pass, return to the bounded multi-branch ports starting with Boyle.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
