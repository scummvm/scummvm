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

- Ported `handle_talk_to_mr_potts @ 0x26000` into a dedicated `engines/harvester/npc/mr_potts_dialogue.*` handler.
  - The confirmed visible handler is compact but stateful: it short-circuits on the blade/Stephanie/digging interruptions, mirrors the evidence-item replies, and preserves the same-day vs later-day followup split keyed by the auxiliary talk-state tail.
  - The known hidden-topic setters for Mr. Potts still live deeper in the overlapping Potts code region, but the engine now covers the top-level dispatch target and its visible branches without guessing at the unrecovered topic loop.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidate is `MRS_POTTS`, which shares the Potts evidence-item shape but also has a recovered overlapping keyword loop worth porting directly.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - After `MRS_POTTS`, the remaining Ghidra-backed handlers are `STEPHANIE`, `SERGEANT`, `SPARKY`, `DAD`, and `BOYLE`.
