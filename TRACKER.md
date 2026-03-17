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

- Ported `handle_talk_to_dwayne @ 0x3a310` into `DialogueSystem` as `engines/harvester/npc/dwayne_dialogue.*`.
  - `DWAYNE` now routes through a dedicated handler instead of the generic stub registry entry, and `DialogueRuntime` now carries the active room name so the native `ST_BEDRM` special case can be mirrored without reintroducing startup-only branching elsewhere.
  - The engine now mirrors the native arrest-priority bark chain, the sheriff-in-diner intro vs outside-sheriff opener split, the delayed Karin-alive next-day follow-up, and the bounded no-sheriff event barks that native gates through the Dwayne-local persisted state block.
  - The item path now mirrors the confirmed shared evidence writes for `LEDGER` / `LEDGER2`, `CASKET_PHOTO` / `CASKET_PHOTOCOPY`, and `PHOTO_OF_WHALEY_HERRILL`, while keeping Dwayne-local state for `BOYLES_BUTTON`, `K_PURSE`, and the unresolved direct reply overrides.
  - The looping keyword/menu path is now explicit in the engine: initial buffer `0x78`, exit topic `0x82`, Whaley/Loomis topic clusters, the Lodge `0x95` response menu on zero-based `dialog.rsp` line `0x96`, and the later `0x98`, `0x9a`, `0x9c`, and `0x9e` rewrites all follow the confirmed native `dialog.rsp` transitions.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Dwayne handler port.

## Next Suggested Action

1. Port `handle_talk_to_edna @ 0x358a0` into `DialogueSystem` as `engines/harvester/npc/edna_dialogue.*`.
   - Edna is the next bounded startup-room handler still behind a stub, and her native item/no-item branches already overlap the confirmed shared evidence helpers used by Dwayne, Mom, Hank, Herrill, and Johnson.
2. After Edna, continue replacing the remaining bounded startup-room stubs before returning to wider helper scans.
   - `handle_talk_to_herrill @ 0x2dc00` and `handle_talk_to_johnson @ 0x27dd0` still need explicit engine-side handlers before their confirmed shared `SHOWN_*` evidence writes can be mirrored.
