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

- Ported `handle_talk_to_stephanie @ 0x36710` into a dedicated `engines/harvester/npc/stephanie_dialogue.*` handler.
  - Stephanie now covers the confirmed top-level dispatch surface: evidence-item replies, the staged intro / revisit flow, the nasty-path FST and action-tag branch, the day-5 and Karin outcome one-shots, and the recovered keyword-menu branches that hang off her visible topic buffers.
  - This completes engine-side coverage for every currently identified top-level NPC dialogue handler recovered from Ghidra.

## Next Suggested Action

1. Switch from NPC coverage work back to dialogue fidelity work.
   - Highest-value followups are Stephanie's deeper hidden topic branches, Boyle's `0x2d620` keyword block, and the separate Mom raw-disassembly rebuild.
2. Keep folding any newly confirmed hidden helper fixes from Ghidra back into the per-NPC handlers.
   - The coverage pass is complete, so remaining work is behavior parity and disassembly cleanup rather than adding more top-level handler classes.
