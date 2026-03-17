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

- Ported `handle_talk_to_sergeant @ 0x30340` into a dedicated `engines/harvester/npc/sergeant_dialogue.*` handler.
  - Native Sergeant dialogue is still a compact quest-delivery handler: the intro/revisit bark pair plus direct item or flag-driven branches for `REMAINS`, `INVITE`, `BARBER_POLE`, `BOLTCLTH`, `SCRATCHED_TUCKER`, `DINER_BURNED`, and `COMPLETED_LODGE_APPLICATION`.
  - This pass also added a minimal runtime `setRuntimeNpcState()` helper so dialogue handlers can mirror native `SET_NPC`-style scene changes, and the shared neutral Sergeant wrapper bits now live in engine-side shared dialogue state for later Stephanie/Dad followups.

## Next Suggested Action

1. Port `handle_talk_to_stephanie @ 0x36710` next.
   - Stephanie is now the last compact-ish named handler before the remaining heavier `DAD` / `BOYLE` hidden-topic ports, and Sergeant's shared-state writes are now in place for the known Tuesday-night / quest followups.
2. After Stephanie, finish the last two Ghidra-identified top-level handlers: `handle_talk_to_dad @ 0x2b020` and `handle_talk_to_boyle @ 0x2cb20`.
   - Revisit the Mom raw-disassembly audit afterward as a separate fidelity pass once the remaining named dispatch targets are no longer stubbed.
