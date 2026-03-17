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

- Ported `handle_talk_to_memb_dir @ 0x396d0` into a dedicated `engines/harvester/npc/memb_dir_dialogue.*` handler.
  - Native Membership Director dialogue is a two-case aftermath handler: before `PC_KILLED_KEWPIE` it always plays `0x2b7` for `MEMB_DIR`, and after that flag it plays the one-shot `0x2d5` line once, then falls silent.
  - The engine now routes `MEMB_DIR` through a concrete handler with the same flag gate and one-shot local state.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next bounded cleanup candidate before the largest trees is `KARIN`, followed by `LOOMIS`, `MOYNAHAN`, `MR_POTTS`, and `MRS_POTTS`.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
