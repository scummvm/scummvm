# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `885`
- Named/documented: `565`
- Still `FUN_*` / undocumented: `320`

## Last Confirmed Action

- Ported `handle_talk_to_johnson @ 0x27dd0` into `DialogueSystem` as `engines/harvester/npc/johnson_dialogue.*`.
  - `JOHNSON` now routes through a dedicated handler instead of the generic stub registry entry, and the engine mirrors the bounded native early short-circuit flags (`2ND_SCRATCH_TUCKER`, `TRY_TO_SCRATCH_TUCKER`, `OPENING_MANHOLE_DAYTIME`), the shared-evidence item replies, the intro gate at `0xd2bf4`, the one-shot no-item bark chain, and the looping keyword menu seeded from zero-based `dialog.rsp` line `0x176`.
  - The previously truncated mid-function block is now recovered from raw bytes: `SCRATCHED_TUCKER` re-arms the intro after `0xc25`, the intro itself is `0xa86` / `0xa8a` / `0xa8e` / `0xa97` / `0xa9b`, and the event bark chain covers `STEPH_MIDGAME_PLAYED`, `KARIN_KIDNAPED`, `KARIN_FOUND_ALIVE`, `BARBER_POLE_STOLEN`, `DINER_BURNED`, `PC_ESCAPED_JAIL`, `GOT_REMAINS_FOR_LODGE`, and `BURNED_TV_STATION`.
  - Johnson's keyword loop is now mirrored in-engine with the native branch grouping: `0x178` / `0x179` opens response line `0x17a` and rewrites to `0x17b`, `0x17c` rewrites to `0x17d`, `0x17e` rewrites to `0x17f`, `0x180` / `0x181` / `0x182`, `0x184` / `0x185`, and `0x187` fall through the generic `0xb69` / `0xbda` exit, and `0x189` loops silently.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Johnson handler port.

## Next Suggested Action

1. Trace the gameplay-side setters for the neutral shared talk-state bits at `0xd2e98`, `0xd2eb0`, and `0xd2eec`.
   - Herrill now reads those bits in-engine on the `GASCAN` path, and Johnson was the last bounded startup-room stub in the same shared-evidence cluster, so the next useful pass is to recover who actually writes those values.
2. After those setters are bounded, continue the wider helper scan around the `0x38230`-`0x38380` dialogue-state wrapper cluster.
   - That range still contains the highest-signal unresolved shared-state shims tied directly to already-confirmed talk-to handlers.
