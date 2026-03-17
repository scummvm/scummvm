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

- Ported `handle_talk_to_loomis @ 0x34f80` into a dedicated `engines/harvester/npc/loomis_dialogue.*` handler.
  - Native Loomis dialogue uses a small two-step no-item state machine: the intro clears the local talk-state block, sets a return-visit followup bit, optionally plays the pre-Dwayne line `0x10fd`, then falls through to `0x1121`; the next no-item talk plays `0x11a8`, and later talks settle on `0x11f6`.
  - The item path is mostly linear but includes one native quirk: `TV_DEED` and `TV_DEED_PHOTOCOPY` collapse into the same note/checkbook helper and `0x1209` line instead of using the usual TV-deed shared flag, and `PHOTO_OF_WHALEY_HERRILL` plays `0x1143` without writing the shared Whaley/Herrill evidence bit.

## Next Suggested Action

1. Move back to the more stateful visible handlers now that the compact visible bucket is mostly cleared.
   - The next best candidates are the larger visible stateful handlers: `MOYNAHAN`, `MR_POTTS`, and `MRS_POTTS`, followed by the heavier hidden-topic handlers.
2. After that visible evidence-handler pass, return to Boyle and the other larger hidden-topic handlers.
   - Boyle still has the best confirmed hidden-topic coverage among the larger remaining NPC handlers.
