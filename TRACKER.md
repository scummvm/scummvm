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

- Ported `handle_talk_to_sparky @ 0x39240` into a dedicated `engines/harvester/npc/sparky_dialogue.*` handler.
  - Native Sparky dialogue is a compact stateful handler: one interrupt bark for the light switch, a first-visit `SPARKY_SPOTS_MEAT` sequence bracketed by `C135.FST` / `C098.FST`, and then a revisit keyword loop with a `0x2c9` submenu rewrite and `NUDETATU.FST` branch.
  - This pass also fixed a Ghidra blind spot by promoting and renaming the tiny FST wrappers at `0x38630`, `0x38650`, `0x38660`, `0x38670`, `0x38690`, and `0x38b80` to `play_c135_fst`, `play_c096_fst`, `play_c096a_fst`, `play_nudetatu_fst`, `play_c098_fst`, and `play_c149_fst`.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidate is `SERGEANT`, which is still compact enough to port directly but needs a couple of runtime NPC/object state helpers.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - After `SERGEANT`, the remaining Ghidra-backed handlers are `STEPHANIE`, `DAD`, and `BOYLE`.
