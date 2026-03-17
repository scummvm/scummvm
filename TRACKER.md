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

- Ported `handle_talk_to_boyle @ 0x2cb20` into a dedicated `engines/harvester/npc/boyle_dialogue.*` handler.
  - Boyle is now covered as a compact visible handler: the one-shot no-item bark, the button / evidence item replies, and the recovered hidden `GASCAN` return exchange that awards `LODGE_APPLICATION`, clears the gas can, and sets `HAVE_LODGE_APP`.
  - The larger hidden Boyle keyword block around `0x2d620` is still documented in `ARCHITECTURE.md` as a follow-on fidelity target, but the top-level dispatch target is no longer stubbed in the engine.

## Next Suggested Action

1. Port `handle_talk_to_stephanie @ 0x36710` next.
   - Stephanie is now the last remaining Ghidra-identified top-level NPC dialogue handler without an engine-side class, and the Sergeant / Potts shared-state prerequisites for her known branches are already in place.
2. After Stephanie lands, switch from coverage work back to fidelity work.
   - Highest-value followups are Stephanie's deeper hidden topic branches, Boyle's `0x2d620` keyword block, and the separate Mom raw-disassembly rebuild.
