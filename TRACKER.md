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

- Ported `handle_talk_to_moynahan @ 0x24a20` into a dedicated `engines/harvester/npc/moynahan_dialogue.*` handler.
  - Native Moynahan dialogue is a stateful visible handler with one early plot bark (`IF_TRY_TO_TAKE_THE_GLUE`), a one-shot no-item intro (`0x3d33`) that falls through to `0x3fd1`, and a mostly linear evidence-item path.
  - The main native quirk is the ledger branch: it reads the shared shown-ledgers state and only plays the extra `0x3f87` line when that shared flag is already set, instead of writing the ledger evidence flag itself; the engine now preserves that behavior explicitly.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidates are the larger visible stateful handlers: `MR_POTTS` and `MRS_POTTS`, followed by the heavier hidden-topic handlers.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
