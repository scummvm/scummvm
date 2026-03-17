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

- Ported `handle_talk_to_herrill @ 0x2dc00` into `DialogueSystem` as `engines/harvester/npc/herrill_dialogue.*`.
  - `HERRILL` now routes through a dedicated handler instead of the generic stub registry entry, and the engine mirrors the bounded native lodge shortcut, the shared evidence item replies, the one-shot no-item bark chain, and the looping keyword menu seeded from zero-based `dialog.rsp` line `0x281`.
  - The previously obscured mid-function branch is now confirmed as `KARIN_KIDNAPED`, not another helper-only condition. Herrill's no-item followups are now implemented directly from raw control flow for `STEPH_MIDGAME_PLAYED`, story day `5`, `SCRATCHED_TUCKER`, `BARBER_POLE_STOLEN`, `KARIN_KIDNAPED`, `KARIN_FOUND_ALIVE`, `KARIN_FOUND_DEAD`, `DINER_BURNED`, and the Whaley/Herrill-photo follow-up.
  - Herrill's `GASCAN` path is now mirrored in-engine as its own response tree: `0x30cf` / `0x30d3` / `0x30d8`, zero-based `dialog.rsp` line `0x280`, the `HAVE_LODGE_APP` split, and the neutral shared talk-state reads at `0xd2e98`, `0xd2eb0`, and `0xd2eec`.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Herrill handler port.

## Next Suggested Action

1. Port `handle_talk_to_johnson @ 0x27dd0` into `DialogueSystem` as `engines/harvester/npc/johnson_dialogue.*`.
   - Johnson is now the last bounded startup-room stub in this shared-evidence cluster, and Herrill's port confirmed the remaining shared talk-state reads around the sheriff/lodge path without changing the broader handler interface.
2. After Johnson, return to the unresolved setters for the neutral shared talk-state bits at `0xd2e98`, `0xd2eb0`, and `0xd2eec`.
   - Herrill now reads those bits in-engine on the `GASCAN` path, but their gameplay-side writers are still outside the currently ported handler set.
