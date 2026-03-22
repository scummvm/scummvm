# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `906`
- Named/documented: `791`
- Remaining `FUN_*`: `115`
- Remaining undocumented total: `115`

## Last Confirmed Action

- On March 22, 2026, re-opened Sergeant's `SCRATCHED_TUCKER` no-item branch against the shipped `DIALOGUE.IDX` and `DIALOG.RSP` assets after ScummVM stopped at Sergeant line `0x420d` without the expected Day 2 response menu.
- Confirmed the missing continuation is asset-backed: Sergeant should continue with `0x421e`, open zero-based `dialog.rsp` line `0x2b4`, and then branch to `0x422a` or `0x422f` / `0x4239`. Patched `engines/harvester/npc/sergeant_dialogue.cpp`, updated the Sergeant dialogue debug dump and `ARCHITECTURE.md`, and rebuilt `build-vscode-harvester-debug/scummvm` successfully.

## Next Suggested Action

- Immediate: rerun the `DAY_2` Sergeant repro in `SERGEANTRM` and confirm ScummVM now shows the recovered `0x2b4` response menu after `0x420d` / `0x421e`, with the accept tail on `0x422a` and the pushback tail on `0x422f` / `0x4239`.
- Follow-up: keep recovering Sergeant's later task-delivery gaps from the same asset-backed evidence, starting with the `BOLT_OF_CLOTH_TAKEN` and `BARBER_POLE_STOLEN` branches, which still collapse early in the current handler.
