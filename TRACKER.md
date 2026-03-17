# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `887`
- Named/documented: `565`
- Still `FUN_*` / undocumented: `322`

## Last Confirmed Action

- Recovered the startup monster-record half of `MONSTERFY` from `HARVEST.SCR` and the matching native room/action logic, then aligned the startup runtime with that confirmed replacement path.
  - Confirmed `CONFIG.INI` routes the town script to `HARVEST.SCR`, and decoding that file with the native `0xaa` XOR shows Jimmy’s restored dialogue chain as `DIAL_JIM_4_D -> START_DIALOG "JIMMY" -> MONST_JIM -> MONSTERFY "JIMMY"`.
  - Confirmed the same decoded script contains both the `NPC "PCHOUSE" ... "JIMMY" "JIMMY_M"` record and a live `MONSTER "PCHOUSE" "JIMMY_M" ... "JIMMY_DEATH_COM"` record, which provides the replacement model path, spawn state, facing, and on-death command tag.
  - Confirmed `dispatch_room_event_actions @ 0x60ee0` recognizes `MONSTERFY` (`case 0x0e`) and `SET_MONSTER` (`case 0x14`), while `room_setup @ 0x73540` separately materializes matching monster records when their room matches and their active/visible state is enabled.
  - ScummVM now parses startup `MONSTER` records, keeps runtime monster state alongside objects/anims/NPCs, materializes active-or-visible room monsters as non-interactive scene actors, supports `SET_MONSTER`, and makes `MONSTERFY` both suppress the original NPC and activate/show the linked `monsterfyTargetName` monster record.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the monster-record runtime changes.

## Next Suggested Action

1. Recover startup monster death / on-death command execution.
   - The decoded `MONSTER` records carry native death action tags such as `JIMMY_DEATH_COM`, and `room_setup`/`MONSTERFY` parity now reaches the replacement spawn, but the startup runtime still has no monster combat/death path to fire those follow-up commands.
2. Confirm whether any startup dialogue/action-tag path needs a value-`2` / outer-exit analogue of `DAT_000d60bc`.
   - Current startup parity still only needs the confirmed value-`1` nested-abort case; the DOS main loop also treats `2` as a high-level exit request, but no startup-script path has been tied to that behavior yet.
