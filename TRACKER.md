# TRACKER

## Current Focus

- Work down the remaining `FUN_*` list in address order, but stop when only literal HMIDRV subcommand IDs or unresolved runtime/x87 structure fields are proven
- Keep renames tied to concrete caller/callee context, strings, DOS/BIOS/DPMI interrupt semantics, and direct buffer/global side effects
- Leave jump-entered dialogue blocks, zero-xref `.image::` fragments, and still-ambiguous runtime/x87 helpers unnamed until the owning structures are reconstructed

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `784`
- Remaining `FUN_*`: `116`
- Remaining undocumented total: `116`

## Last Confirmed Action

- On March 19, 2026, ran a manual desktop validation pass against the live in-room ESC overlay with a persistent ScummVM GUI session and `osascript` keyboard input: confirmed the six-row room menu is reachable and keyboard-driven, reproduced a real palette-return regression on `SAVE GAME -> Esc` / `LOAD GAME -> Esc`, and fixed it by reapplying the room palette whenever `runRoomMenuStub` redraws its backdrop after a submenu returns.
- Rebuilt the touched Harvester binary after that room-menu palette fix: `engines/harvester/menu.o` and `scummvm`, then revalidated that both `SAVE GAME -> Esc` and `LOAD GAME -> Esc` return to the room-menu overlay without the corrupted palette.
- On March 19, 2026, audited the legitimate `run_main_menu` callers against the live `HARVEST.LE` Ghidra session instead of the cold-start path: confirmed that the six-row in-room ESC overlay belongs to the room-menu path, while the true `run_main_menu` return/death path blanks `SAVE GAME` and `LOAD GAME` unless an active room session is still live.
- Applied that parity fix in `engines/harvester/menu.cpp` / `engines/harvester/menu.h`: `runMainMenuStub` now filters `SAVE GAME` and `LOAD GAME` out of the true main-menu stub when there is no current startup save-room state, while `runRoomMenuStub` continues to expose the full six-row in-room menu.
- Rebuilt the touched Harvester objects successfully after the caller-audit fix: `engines/harvester/menu.o`, `engines/harvester/flow.o`, `engines/harvester/room.o`, and `engines/harvester/harvester.o`.
- On March 19, 2026, fixed the cold-start regression from the previous menu pass: `Flow::run()` no longer jumps from the initial quick-tips overlay into `run_main_menu`, and instead restores the confirmed native cold-start path back into `runRoomLoop("START")`. The real `run_main_menu` work remains limited to its live main-loop / game-over entry points, while the new `LOAD GAME` UI and `NEW GAME` confirmation support stay available on those legitimate menu paths.
- Rebuilt the touched Harvester object successfully after that correction: `engines/harvester/flow.o`.
- On March 19, 2026, completed the first real `run_main_menu` / room-menu parity pass in `engines/harvester`: `LOAD GAME` now uses the native `LOADGAME.BM` / `LOADGAME.PAL` slot picker; `NEW GAME` now uses the confirmed `MENU.INI:newgame` confirmation prompt in both the legitimate top-level menu path and the in-room ESC menu; and the in-room ESC menu now unwinds cleanly into restart/load handoffs instead of leaving those rows as stubs.
- Rebuilt the touched Harvester objects successfully: `engines/harvester/menu.o`, `engines/harvester/flow.o`, `engines/harvester/room.o`, and `engines/harvester/harvester.o`.
- On March 19, 2026, audited the current `engines/harvester` implementation against the live `HARVEST.LE` Ghidra session and the confirmed notes in `ARCHITECTURE.md`; verified that boot/resource loading, room setup, inventory, dialogue, save menu, help, options, and quit confirmation are already implemented, while the main remaining reimplementation gaps are startup/room menu parity, actor runtime/combat/timers, and wider script opcode coverage.
- Finished another rename pass, saved `HARVEST.LE`, and advanced the current tracker cycle to 194 confident renames total.
- The latest two passes resolved 12 more high-confidence names:
  `execute_runtime_handler_list_until_target`, `find_and_store_runtime_handler_type3_entry`,
  `convert_int32_to_extended`, `convert_double_words_to_extended`, `convert_extended_to_float_bits`, `convert_extended_to_double_words`,
  `convert_float_bits_to_extended`, `round_extended_by_control_word_mode`,
  `compute_extended_atan2`, `compute_extended_atan_core`, `compute_extended_log2`, and `compute_extended_natural_log`.
- The current boundary is now narrower:
  `FUN_00021100` / `FUN_00021120` are still only proven as paired min/max extent updaters with unresolved owner-axis semantics;
  `FUN_0002e3b4`, `FUN_0002e41b`, `FUN_0002e46a`, `FUN_0002f7fd`, `FUN_000324a2`, and `FUN_0003351b` are still jump-entered dialogue mis-splits rather than stable standalone functions;
  `FUN_00083fb3`, `FUN_00084a8b`, `FUN_00084b6b`, and the `FUN_00087005` through `FUN_00087439` band are still HMIDRV/sound-bank wrappers where only subcommand IDs and buffer movement are proven;
  the remaining `FUN_0008c3d8` through `FUN_00091518` cluster is now mostly x87/no-87 runtime state, shared polynomial helpers, operand classifiers, and throw/unwind support whose table semantics, hidden ABI, or record layouts are still below the rename threshold;
  the leading `.image::` entries remain zero-xref fragments with no reliable subsystem context.

## Next Suggested Action

1. Continue the manual desktop validation pass from the live GUI session: finish spot-checking the remaining in-room ESC rows (`NEW GAME`, `OPTIONS`, `HELP`, `QUIT GAME`), then find a reproducible `GODEATHFLIC` / game-over trigger or suitable save so the legitimate `run_main_menu` path can be reached and confirmed to show only `NEW GAME`, `OPTIONS`, `HELP`, and `QUIT GAME`.
2. Continue the remaining top-level/main-title rendering parity only on the true `run_main_menu` callers from `run_harvester_main_loop` / `run_game_over_screen`; the cold-start quick-tips exit and the room ESC overlay are now separated from that work.

## Reimplementation Priority Order

1. Finish startup and room menu parity.
   Files: `engines/harvester/menu.cpp`, `engines/harvester/menu.h`, `engines/harvester/flow.cpp`, `engines/harvester/harvester.cpp`, `engines/harvester/harvester.h`.
   Native anchors: `run_main_menu @ 0x67390`, `run_load_game_menu @ 0x64910`, `run_save_game_menu @ 0x632c0`, `run_controls_help_screen @ 0x6c3e0`.
   Concrete tasks: keep the new real `LOAD GAME` path and `NEW GAME` / restart confirmation aligned with the confirmed native notes; do not route the cold-start quick-tips exit through `run_main_menu`; finish the remaining true `run_main_menu` rendering/timer/palette restore details only on its legitimate live callers.
   Exit criteria: every visible menu row performs real work instead of falling back to a debug stub, and the startup-menu return path plus the in-room ESC menu expose the same native actions.
2. Implement the actor runtime and combat-adjacent substrate.
   Files: `engines/harvester/room.cpp`, `engines/harvester/flow.cpp`, `engines/harvester/runtime_entity.cpp`, `engines/harvester/runtime_entity.h`, `engines/harvester/script.cpp`, `engines/harvester/script.h`.
   Native anchors: `spawn_player_combat_avatar @ 0x54220`, `update_player_combat_avatar_state @ 0x553a0`, `teardown_player_combat_avatar @ 0x55010`, `tick_monster_entity_runtime @ 0x54140`, `spawn_timer_entity_from_record @ 0x59390`, `pause_timer_entity_countdowns @ 0x80460`, `resume_timer_entity_countdowns @ 0x804a0`.
   Concrete tasks: separate passive scene visuals from live actor state; persist the player combat-avatar HP/loadout state instead of only the coarse room-placement snapshot; add monster update hooks; add timer countdown bookkeeping needed by help/menu pauses and room events.
   Exit criteria: monsters are no longer static scene art only, and timer-sensitive menu/help behavior matches the confirmed native flow.
3. Widen town-script action and room-event coverage only where native behavior is already confirmed.
   Files: `engines/harvester/script.cpp`, `engines/harvester/room.cpp`, `engines/harvester/dialogue.cpp`.
   Native anchors: `dispatch_room_event_actions @ 0x60ee0`, `run_town_script_interpreter @ 0x46d80`, and the confirmed town-script notes in `ARCHITECTURE.md`.
   Concrete tasks: drive missing work from broken rooms and interactions; add only the opcodes and side effects needed to unblock verified room paths; keep unsupported commands explicit rather than guessing.
   Exit criteria: room progression failures shrink to a small, explicit list of still-unimplemented native commands instead of broad "unsupported" fallthrough.
4. Tighten inventory and item-use flow against that actor runtime.
   Files: `engines/harvester/inventory.cpp`, `engines/harvester/room.cpp`, `engines/harvester/script.cpp`.
   Native anchors: `run_inventory_screen @ 0x7df10`, `add_object_to_inventory @ 0x7c8b0`, `sync_player_combat_weapon_resource_icons @ 0x792c0`, and the confirmed inventory carry/use notes in `ARCHITECTURE.md`.
   Concrete tasks: keep the current carry/use handoff model, but recheck it once actor runtime state, HP, timers, and combat-adjacent HUD elements are live; fold any missing status/icon behavior in here rather than earlier.
   Exit criteria: inventory, carried items, and room-target use cases all operate against the same live actor state that room gameplay uses.
5. Leave low-level runtime, HMIDRV, and x87 cleanup as deferred RE work unless they block a confirmed gameplay feature.
   Ghidra anchors: the remaining unwind helpers around `FUN_0008f64b` through `FUN_000905df`, the HMIDRV wrapper band around `FUN_00087005`, and the x87/no-87 runtime cluster beginning near `FUN_0008c3d8`.
   Rationale: those clusters still matter for naming completeness, but the current engine-side gaps are better bounded and have higher reimplementation value.
