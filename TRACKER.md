# TRACKER

## Current Focus

- Implement the remaining Ghidra-identified NPC dialogue handlers in the engine
- Keep each NPC port scoped to confirmed native behavior only
- Update `TRACKER.md` and commit after each completed NPC

## Progress

- Program: `HARVEST.LE`
- Total functions: `896`
- Named/documented: `577`
- Still `FUN_*` / undocumented: `319`

## Last Confirmed Action

- Ported `handle_talk_to_dad @ 0x2b020` into a dedicated `engines/harvester/npc/dad_dialogue.*` handler.
  - Dad is now covered as a compact visible handler: the `TAKING_BONDAGE` interrupt, the two-state no-item bark path, and the linear evidence-item fan-out for corpse photo, Whaley/Herrill photo, note/checkbook evidence, and `MEAT_PERMISSION0`.
  - The only still-unrecovered piece in that handler is the gameplay-side setter behind Dad's local `MEAT_PERMISSION0` wrapper at `0x38400`; the engine keeps that state explicit instead of guessing a write path.

## Next Suggested Action

1. Port `handle_talk_to_stephanie @ 0x36710` next.
   - Stephanie is now the only remaining large visible/topic-heavy handler before the final compact Boyle pass.
2. After Stephanie, finish the last remaining non-stub top-level handler: `handle_talk_to_boyle @ 0x2cb20`.
   - Revisit the Mom raw-disassembly audit afterward as a separate fidelity pass once `STEPHANIE` and `BOYLE` are both in place.
