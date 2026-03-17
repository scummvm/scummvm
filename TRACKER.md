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

- Ported `handle_talk_to_edna @ 0x358a0` into `DialogueSystem` as `engines/harvester/npc/edna_dialogue.*`.
  - `EDNA` now routes through a dedicated handler instead of the generic stub registry entry, and the runtime script API now exposes the two minimal object-state helpers this native handler actually needs: add `REWARD_MONEY` to inventory and reveal `DNAEXT/SIGNOUT`.
  - The engine now mirrors the native early special branches: `KILLED_KARIN1` -> `0x3cdf`, `DNA_S_SUICIDE_NOTE` -> `C043.FST` / `C043K.FST` plus `SIGNOUT` visibility, and `BRING_KARIN_TO_SHERIFF` -> `0x3cce` / `0x3cd2` / `0x3cd7` plus the `REWARD_MONEY` inventory award and flag clear.
  - The first normal no-item visit now mirrors the native intro gate on `g_edna_talk_state_block`: `0x3a95`, zero-based `dialog.rsp` line `0x9f`, then response `1` -> `0x3aa1` / `0x3aa6` / `0x3aaa` / `0x3aae` / `0x3ac6` with shared `d2f04` set, response `2` -> `0x3ab2` / `0x3ab7` / `0x3abb` / `0x3ac2` / `0x3ac6`.
  - The item path now mirrors the confirmed shared evidence writes for casket photo, dual-ledger, Whaley/Herrill, note/checkbook, and TV deed evidence, while keeping Edna-local state for the `K_PURSE` one-shot line.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Edna handler port.

## Next Suggested Action

1. Port `handle_talk_to_herrill @ 0x2dc00` into `DialogueSystem` as `engines/harvester/npc/herrill_dialogue.*`.
   - Herrill is now the next bounded startup-room handler still behind a stub, and his visible evidence branches overlap the same shared `SHOWN_*` note/checkbook, TV deed, casket photo, and Whaley/Herrill helpers Edna just confirmed in-engine.
2. After Herrill, continue with `handle_talk_to_johnson @ 0x27dd0`.
   - Johnson is the last remaining bounded startup-room stub in this evidence-sharing cluster before the tracker can return to wider helper scans.
