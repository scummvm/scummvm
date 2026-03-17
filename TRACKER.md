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

- Ported `handle_talk_to_wasp_woman @ 0x2fde0` into `startup_dialogue.cpp`.
  - The engine now mirrors Wasp Woman's item branches for `PHOTO_OF_WHALEY_HERRILL`, `CASKET_PHOTO` / `CASKET_PHOTOCOPY`, and the note/checkbook evidence group, including the shared `SHOWN_*` runtime-flag writes before `0x4ca6` / `0x4cae` / `0x4cb2` / `0x4cb6`, `0x4cbd`, and `0x4cc3`.
  - The no-item path now mirrors the native one-time intro sequence (`0x4bee`, optional `0x4bf2`, `0x4bf6`, `0x4bfc`), the fixed `0x301` keyword menu seed, and the `0x302`, `0x305`, `0x308`, `0x30a`, `0x30c`, and `0x30e` menu branches.
  - The query-only `get_set_wasp_woman_dialogue_state_d2f10 @ 0x38420` gate is represented as shared engine dialogue state; it currently stays false because the native audit has not recovered any startup-side writer beyond the global reset path, so the optional `0x4bf2` line and the `0x307` response-menu follow-up remain correctly locked behind that unresolved state.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Wasp Woman dialogue port.

## Next Suggested Action

1. Port `handle_talk_to_pta_mom @ 0x34e30` into `startup_dialogue.cpp`.
   - The handler is already bounded to `PTA_MOM1` through `PTA_MOM5`, uses the fixed `0x297` response menu, and is the next explicit startup-room talk cluster after Wasp Woman.
2. After PTA Mom, continue porting bounded startup-room handlers before returning to wider helper scans.
   - Dwayne, Edna, Herrill, and Johnson still need explicit engine-side room handlers before their confirmed shared `SHOWN_*` evidence writes can be mirrored.
