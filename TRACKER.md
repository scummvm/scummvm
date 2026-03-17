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

- Ported `handle_talk_to_pta_mom @ 0x34e30` into `DialogueSystem` as `engines/harvester/npc/pta_mom_dialogue.*`.
  - `PTA_MOM1` through `PTA_MOM5` now route through a dedicated handler instead of the generic stub registry entry; the plain `PTA_MOM` token remains stubbed because this pass did not confirm it as a native talk-table alias.
  - The engine now mirrors the native `PTA_RESPOND_TO_TV` one-shot override: `0x3233` with `PTA_MOM1` head variant `2`, then the flag is cleared and the handler returns without opening the response menu.
  - The normal path now mirrors the compact native exchange: one random opener from `0x31ee`, `0x31f2`, or `0x31f6`, then zero-based `dialog.rsp` line `0x297`, with response `1` -> `0x3204` / `0x320b`, response `2` -> `0x320f` / `0x3213`, and response `3` -> `0x3217` / `0x321c` / `0x3221` / `0x3226`.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the PTA Mom handler port.

## Next Suggested Action

1. Port `handle_talk_to_dwayne @ 0x3a310` into `DialogueSystem` as `engines/harvester/npc/dwayne_dialogue.*`.
   - Dwayne is the next explicit startup-room handler still behind a stub, and the native audit already ties him to confirmed shared evidence helpers such as `get_set_discussed_note_checkbook_evidence`, `get_set_discussed_casket_photo_evidence`, `get_set_discussed_whaley_herrill_photo`, and the Dwayne-local `K_PURSE` / Karin-follow-up state wrappers.
2. After Dwayne, continue replacing the remaining bounded startup-room stubs before returning to wider helper scans.
   - Edna, Herrill, and Johnson still need explicit engine-side room handlers before their confirmed shared `SHOWN_*` evidence writes can be mirrored.
