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

- Ported `handle_talk_to_nude_man @ 0x2d9b0` into a dedicated `engines/harvester/npc/nude_man_dialogue.*` handler.
  - Native Nude Man dialogue is a compact three-way branch: `DAY_FLAG` forces `0x65c` for `NUDE_MAN`, the first non-day conversation clears the local talk-state block and plays `0x606` for `PC`, and later non-day conversations fall through to `0x651` for `PC`.
  - The engine now routes `NUDE_MAN` through a concrete handler with matching day-gate behavior, local talk-state, and PC-spoken follow-up lines.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next bounded cleanup candidates before the largest trees are `MEMB_DIR` and `KARIN`, followed by `LOOMIS`, `MOYNAHAN`, `MR_POTTS`, and `MRS_POTTS`.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
