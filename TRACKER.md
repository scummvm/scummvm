# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `890`
- Named/documented: `566`
- Still `FUN_*` / undocumented: `324`

## Last Confirmed Action

- Refactored `StartupDialogueSystem` into `DialogueSystem` and split room-NPC handlers into `engines/harvester/npc/`.
  - `startup_dialogue.cpp` / `startup_dialogue.h` were renamed to `dialogue.cpp` / `dialogue.h`, and `StartupFlow` now owns `DialogueSystem _dialogue`.
  - Jimmy, Mom, Hank, and Wasp Woman now dispatch through dedicated handler classes in `engines/harvester/npc/`, while `PTA_MOM`, `PTA_MOM1`-`PTA_MOM5`, `DWAYNE`, `EDNA`, `HERRILL`, and `JOHNSON` are stubbed through the same registry.
  - The runtime split preserves the existing recovered dialogue behavior while making the next startup-room ports land as isolated NPC handlers instead of growing a single monolithic dispatcher again.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the refactor.

## Next Suggested Action

1. Port `handle_talk_to_pta_mom @ 0x34e30` into `DialogueSystem` as `engines/harvester/npc/pta_mom_dialogue.*`.
   - The handler is already bounded to `PTA_MOM1` through `PTA_MOM5`, uses the fixed `0x297` response menu, and now has an explicit stub slot in the NPC registry to replace.
2. After PTA Mom, continue porting bounded startup-room handlers before returning to wider helper scans.
   - Dwayne, Edna, Herrill, and Johnson still need explicit engine-side room handlers before their confirmed shared `SHOWN_*` evidence writes can be mirrored.
