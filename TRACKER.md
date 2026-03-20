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

- On March 20, 2026, rescanned every native startup-room NPC item branch against the decoded `HARVEST.SCR` `USEITEM` table instead of assuming that every NPC-targeted action tag still needed separate ScummVM plumbing.
- Confirmed there are only three NPC-targeted `USEITEM` records in the shipped script: `INV_MAG -> LOOMIS`, `BOYLES_BUTTON -> BOYLE`, and `GASCAN -> BOYLE`. Ghidra plus the current `handle_talk_to_boyle` port showed Boyle's two cases already carry their native item-side effects inside the dialogue handler, while Loomis was the only remaining script-side room-state gap.
- No additional engine-side behavior changes were justified after that audit pass; the earlier Loomis patch remains the only required NPC-targeted `USEITEM` fix, and the next safe step is live validation rather than more speculative handler changes.
- On March 20, 2026, compared the ScummVM Loomis item-use path against both native `handle_talk_to_loomis @ 0x34f80` in Ghidra and the decoded `HARVEST.SCR` sheriff-office records instead of guessing from dialogue alone.
- Confirmed that the native Loomis talk handler already matches the ScummVM dialogue lines for `INV_MAG`, but the room script also defines `USEITEM "INV_MAG" "SHRFOFC" "LOOMIS" "GO_LOOMISA"`; that action sets `GAVE_MAG_TO_LOOMIS_TODAY` and immediately `SET_NPC "LOOMIS" "F" "F"`, which explained why ScummVM kept Loomis in the room after the magazine handoff.
- Patched `engines/harvester/npc/loomis_dialogue.cpp` to apply those confirmed room-script side effects during the Loomis magazine branch and queue a mutated-runtime refresh so the live sheriff-office scene drops Loomis after the dialogue, then rebuilt `scummvm` successfully.
- On March 20, 2026, tightened the current integrated inventory / room-item handoff against the persisted actor runtime instead of inventing a separate inventory state machine: right-clicking confirmed weapon inventory items now toggles the saved player combat loadout ids recovered from the native inventory screen, the open inventory overlay now refreshes its `INV_STAT*` portrait immediately when live player HP changes, and secondary-click while an inventory item is actively selected now cancels that carry/use latch without forcibly closing the panel.
- Kept the current ScummVM-side carry/use handoff model intact, but bounded it more tightly to the same live actor state that save/load and timer-driven room actions already use: inventory weapon toggles write through `Script::setPlayerCombatLoadout`, and the room loop captures that updated runtime state immediately for later save/menu handoffs.
- Rebuilt the touched Harvester objects successfully after that inventory/runtime parity pass: `engines/harvester/inventory.o`, `engines/harvester/room.o`, and `scummvm`.
- On March 19, 2026, widened the confirmed town-script / room-event coverage in `engines/harvester` using only opcode behavior already bounded in Ghidra and `ARCHITECTURE.md`: added `CHECK_PERC`, `EXEC_LIST`, `GOFLIC`, `SHOW_TEXT`, `PC_GOTO_XZ`, and `CHANGE_LIGHTING` to the startup-script dispatcher, parsed `EXEC_LIST` records from `HARVEST.SCR`, and threaded the new deferred action results through room interactions plus queued dialogue follow-ups instead of dropping them as generic unsupported fallthrough.
- The room loop now applies confirmed deferred side effects in native order where applicable: `GOFLIC` plays FST cutscenes then continues with the chained action tag, `SHOW_TEXT` runs a blocking room-modal textbox using the existing inspect-text resources, `PC_GOTO_XZ` repositions the live player avatar on the current room depth line, and `CHANGE_LIGHTING` covers the confirmed `DIM`, `NORMAL`, and `NONE` modes while keeping `FADE_IN` explicit as a still-unbounded no-op.
- Rebuilt the touched Harvester objects successfully after that script/event coverage pass: `engines/harvester/script.o`, `engines/harvester/room.o`, `engines/harvester/dialogue.o`, and `scummvm`.
- On March 19, 2026, implemented the first actor-runtime / combat-adjacent substrate pass in `engines/harvester`: parsed and persisted richer `MONSTER` plus `TIMER` records from `HARVEST.SCR`, bumped startup-save serialization to version `2` with backward-compatible load fallback, preserved player combat HP/loadout/control-pause state in the startup runtime snapshot, spawned non-visual timer entities during room setup, and paused/resumed their countdowns across room setup fades and the true `run_main_menu` path.
- Extended the live room loop to sync timer and monster runtime state back into the startup snapshot before save/refresh handoffs, dispatch expired room timers through the confirmed town-script action-tag path, and promote room monsters from static single-frame props into live runtime entities with facing-based idle/walk frame bands.
- Rebuilt the touched Harvester objects successfully after the actor-runtime/timer pass: `engines/harvester/script.o`, `engines/harvester/runtime_entity.o`, `engines/harvester/flow.o`, `engines/harvester/room.o`, `engines/harvester/menu.o`, `engines/harvester/harvester.o`, and `scummvm`.
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

1. Run a live Harvester validation pass that covers the sheriff-office magazine handoff plus Boyle's button/gascan item branches, confirming that the Loomis disappearance now lands correctly and that the already-ported Boyle logic still matches the native outcomes in gameplay.
2. Run a manual desktop validation pass that specifically covers the tightened inventory/runtime hooks: verify live HP changes update the open inventory `INV_STAT*` portrait immediately, right-clicking weapon items toggles the persisted combat loadout without desynchronizing save/load state, and closing/reopening the inventory preserves carried-item handoff semantics.
3. If that pass exposes any remaining inventory gaps, keep them narrowly bounded to confirmed native behaviors only. The next likely candidates are the room HUD weapon-resource icon strip and any still-missing inventory secondary-click item actions beyond the now-confirmed weapon-loadout path.

## Reimplementation Priority Order

1. Validate and finish the actor runtime and combat-adjacent substrate.
   Files: `engines/harvester/room.cpp`, `engines/harvester/flow.cpp`, `engines/harvester/runtime_entity.cpp`, `engines/harvester/runtime_entity.h`, `engines/harvester/script.cpp`, `engines/harvester/script.h`.
   Native anchors: `spawn_player_combat_avatar @ 0x54220`, `update_player_combat_avatar_state @ 0x553a0`, `teardown_player_combat_avatar @ 0x55010`, `tick_monster_entity_runtime @ 0x54140`, `spawn_timer_entity_from_record @ 0x59390`, `pause_timer_entity_countdowns @ 0x80460`, `resume_timer_entity_countdowns @ 0x804a0`.
   Concrete tasks: validate the new timer pause/resume bookkeeping and timer-to-action dispatch in a live room; confirm the true `run_main_menu` pause path now freezes room countdowns; if the live monster rooms still need more than the new facing/animation hooks, extend only the proven actor state transitions from the native anchors.
   Exit criteria: room timers survive save/load and pause correctly across menus/help, and the remaining monster/combat gaps are narrowed to explicit missing native state-machine branches instead of static-scene placeholders.
2. Validate the widened town-script action and room-event coverage, then only implement any remaining script-side gaps that are still explicitly proven.
   Files: `engines/harvester/script.cpp`, `engines/harvester/room.cpp`, `engines/harvester/dialogue.cpp`.
   Native anchors: `dispatch_room_event_actions @ 0x60ee0`, `run_town_script_interpreter @ 0x46d80`, and the confirmed town-script notes in `ARCHITECTURE.md`.
   Concrete tasks: exercise the now-supported `CHECK_PERC`, `EXEC_LIST`, `GOFLIC`, `SHOW_TEXT`, `PC_GOTO_XZ`, and `CHANGE_LIGHTING` paths in live rooms; only if validation still fails, add the exact missing command or side effect proven by that room.
   Exit criteria: the remaining room progression failures reduce to a short, named list of still-unimplemented native commands or side effects, with `CHANGE_CD` / `FADE_IN` either confirmed as blockers or explicitly ruled out.
3. Validate the tightened inventory and item-use flow against live gameplay, then only extend the remaining confirmed inventory-specific behavior that still blocks parity.
   Files: `engines/harvester/inventory.cpp`, `engines/harvester/room.cpp`, `engines/harvester/script.cpp`.
   Native anchors: `run_inventory_screen @ 0x7df10`, `add_object_to_inventory @ 0x7c8b0`, `sync_player_combat_weapon_resource_icons @ 0x792c0`, and the confirmed inventory carry/use notes in `ARCHITECTURE.md`.
   Concrete tasks: validate the new live HP portrait refresh plus weapon-loadout toggles, then decide from a desktop pass whether the remaining native inventory-specific gaps are limited to HUD icon strip restoration or broader item secondary-click actions.
   Exit criteria: any remaining inventory parity work is reduced to a short, explicit list of still-unimplemented native inventory behaviors instead of a broad actor-runtime mismatch.
4. Leave low-level runtime, HMIDRV, and x87 cleanup as deferred RE work unless they block a confirmed gameplay feature.
   Ghidra anchors: the remaining unwind helpers around `FUN_0008f64b` through `FUN_000905df`, the HMIDRV wrapper band around `FUN_00087005`, and the x87/no-87 runtime cluster beginning near `FUN_0008c3d8`.
   Rationale: those clusters still matter for naming completeness, but the current engine-side gaps are better bounded and have higher reimplementation value.
