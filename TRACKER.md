# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `908`
- Named/documented: `791`
- Remaining `FUN_*`: `117`
- Remaining undocumented total: `117`

## Last Confirmed Action

- On March 22, 2026, re-opened Sergeant's `SCRATCHED_TUCKER` no-item branch against the shipped `DIALOGUE.IDX` and `DIALOG.RSP` assets after ScummVM stopped at Sergeant line `0x420d` without the expected Day 2 response menu.
- Confirmed the missing continuation is asset-backed: Sergeant should continue with `0x421e`, open zero-based `dialog.rsp` line `0x2b4`, and then branch to `0x422a` or `0x422f` / `0x4239`. Patched `engines/harvester/npc/sergeant_dialogue.cpp`, updated the Sergeant dialogue debug dump and `ARCHITECTURE.md`, and rebuilt `build-vscode-harvester-debug/scummvm` successfully.
- On March 22, 2026, cleaned up the native Sergeant room handler in Ghidra by rebuilding `handle_talk_to_sergeant` at `0x30340` from a broken one-byte stub into its real front-half body, setting an explicit `void __stdcall` prototype, and renaming the Sergeant-local `0xd2d50..0xd2d70` state slots to stable labels. The only remaining decompiler noise in that front half is the known register-lift artifact on `ASSIGNED_DNA_TASK` and the odd extra register shown on the `BOLTCLTH` branch.
- On March 22, 2026, recovered the remaining in-line Sergeant continuation heads from native control flow instead of raw asset chasing: labeled and commented `sergeant_completed_application_followup` (`0x30c42`), `sergeant_intro_or_revisit_dispatch` (`0x30d7e`), `sergeant_first_contact_intro_sequence` (`0x30d8b`), `sergeant_revisit_reminder_sequence` (`0x30eff`), and the shared `sergeant_day5_exit_check` / `sergeant_day5_exit_branch` (`0x30f99` / `0x30fa7`). The plate comment on `handle_talk_to_sergeant` now points directly at those continuation blocks.
- On March 22, 2026, reapplied the recovered Sergeant `SCRATCHED_TUCKER` continuation to ScummVM: after the `0x2b4` response menu, both the `0x422a` accept tail and the `0x422f` / `0x4239` pushback tail now fall through a one-shot local `dialogueStateD2d58` gate that dispatches `GO_DAY_3`, matching the native `0x30b84..0x30ba6` action-tag handoff. Updated `engines/harvester/npc/sergeant_dialogue.cpp`, `engines/harvester/dialogue_debug.cpp`, `ARCHITECTURE.md`, and rebuilt `build-vscode-harvester-debug/scummvm` successfully.

## Next Suggested Action

- Immediate: rerun the `SERGEANTRM` `SCRATCHED_TUCKER` repro and confirm the `0x2b4` menu now falls through into the native `GO_DAY_3` handoff, including the expected end-of-day FST/cutscene transition after either response branch.
- Follow-up: if the day transition now matches native, return to the remaining Sergeant signature cleanup in Ghidra so the `ASSIGNED_DNA_TASK` and `BOLTCLTH` branches decompile as plain literal `1` writes.
