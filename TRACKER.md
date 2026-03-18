# TRACKER

## Current Focus

- Re-test Jimmy's first no-item conversation in-engine against DOSBox now that the hidden response-menu chain is wired up
- Verify any remaining NPC openings that were decompiled as single-line returns against raw post-`play_dialogue_line` blocks in Ghidra
- Keep dialogue handler edits tied to confirmed `DIALOG.RSP` line indices and native state writes instead of inferred subtitle order

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited `handle_talk_to_jimmy`, `run_dialogue_response_menu`, and the hidden post-`play_dialogue_line` Jimmy blocks in Ghidra after DOSBox showed the engine was stopping too early.
  - Jimmy still has no keyword loop, but his first no-item conversation is not a single bark: native code plays `0x4a4c` / `0x4a58`, sets `PAPER_CHK_1` and `GIVEN_PAPER_TODAY`, then opens zero-based `DIALOG.RSP` response menus `0xf5`, `0xf6`, and `0xf7` in sequence.
  - The third response-menu branch checks inventory `SNEAKERS`, restores them to `RAH`, and awards `BROOMKEY`; the direct `SNEAKERS` and first-time `NEWSPAPER` item paths also continue into extra native follow-up lines that the earlier decompile had hidden.
- Updated `engines/harvester/npc/jimmy_dialogue.cpp` to follow that native response order and rebuilt `engines/harvester/npc/jimmy_dialogue.o` successfully.

## Next Suggested Action

1. Re-test Jimmy's first conversation from a fresh state and confirm the response menus appear in the native `0xf5 -> 0xf6 -> 0xf7` order with the same follow-up lines as DOSBox.
2. If another NPC opener still stops after one line, inspect its raw xrefs to `run_dialogue_response_menu` and `load_dialogue_response_line` before trusting the decompiler output.
