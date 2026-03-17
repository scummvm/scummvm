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

- Ported `handle_talk_to_mrs_potts @ 0x2ee70` into a dedicated `engines/harvester/npc/mrs_potts_dialogue.*` handler.
  - The confirmed top-level handler now mirrors Mrs. Potts's shared evidence-item replies, her Stephanie-dead room split, and the auxiliary return-visit / event bark tail.
  - Ghidra still exposes the `FUN_0002f7fd` overlapping keyword-loop region separately from the top-level visible flow, so that menu block remains documented as an overlapping hidden branch until a trustworthy engine entry path is recovered.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidates are the remaining compact named handlers: `SPARKY` and `SERGEANT`, before the larger hidden-topic-heavy ports.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - After `SPARKY` and `SERGEANT`, the remaining Ghidra-backed handlers are `STEPHANIE`, `DAD`, and `BOYLE`.
