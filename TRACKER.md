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

- Confirmed the startup value-`2` / outer-exit analogue is reached through `GODEATHFLIC`, then aligned the startup runtime with that native game-over return-to-menu path.
  - Confirmed `parse_command_record @ 0x5e410` maps `GODEATHFLIC` to opcode `0x1c`, and `dispatch_room_event_actions @ 0x60ee0` handles that case by clearing the current palette, playing the supplied FST path, calling `run_game_over_screen()`, restoring music volume, and returning immediately.
  - Confirmed `run_game_over_screen @ 0x7c540` writes `DAT_000d60bc = (run_main_menu() == -1) + 1`, so the same native path carries both the already-implemented value-`1` nested-abort case and the value-`2` high-level exit-to-menu case.
  - Confirmed the decoded `HARVEST.SCR` actually uses that route in the blood-drive chain: `BLOOD_DRIVE_A -> CHANGE_LIGHTING NONE -> BLOOD_DRIVE_B -> GOFLIC GRAPHIC\\FST\\EVENING.FST -> BLOOD_DRIVE_C -> GODEATHFLIC GRAPHIC\\FST\\C006.FST`.
  - ScummVM now treats `GODEATHFLIC` as a terminal startup interaction result, plays the death FST, marks a pending main-menu return, and unwinds recursive room/dialogue continuations without resuming the abandoned room chain or spawning nested menu loops.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the `GODEATHFLIC` / outer-exit work.

## Next Suggested Action

1. Recover startup `GOFLIC` and `CHANGE_LIGHTING` handling for the same confirmed blood-drive sequence.
   - The native `GODEATHFLIC` / value-`2` path is now covered, but the decoded `BLOOD_DRIVE_*` chain still depends on the preceding `CHANGE_LIGHTING NONE` and `GOFLIC GRAPHIC\\FST\\EVENING.FST` opcodes before it reaches `C006.FST`.
2. Revisit startup monster death / on-death command execution once there is a safe trigger path.
   - The decoded `MONSTER` records still carry native death action tags such as `JIMMY_DEATH_COM`, but startup parity still lacks a non-speculative monster combat/death state transition to fire them.
