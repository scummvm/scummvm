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

- Ported `handle_talk_to_karin @ 0x2ea90` into a dedicated `engines/harvester/npc/karin_dialogue.*` handler.
  - Native Karin dialogue is a compact stateful handler: the CEM10 alive line (`0x10a5`) and the pre-rescue default line (`0x1076`) are both one-shot PC-spoken branches, the item path handles casket/photo evidence and `INV_MAG`, and an extra photo-reply override bit suppresses native evidence writes when set.
  - The engine now routes `KARIN` through a concrete handler with matching one-shot state and evidence replies; the photo-reply override remains explicit local state until its gameplay-side setter is recovered.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidates are the larger visible stateful handlers: `LOOMIS`, `MOYNAHAN`, `MR_POTTS`, and `MRS_POTTS`, followed by the heavier hidden-topic handlers.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
