# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `910`
- Named/documented: `594`
- Still `FUN_*` / undocumented: `316`

## Last Confirmed Action

- Rebuilt `handle_talk_to_moynahan @ 0x24a20` from the corrected native flow and renamed the recovered shared wrappers at `0x382e0` / `0x38300`.
  - Moynahan now preserves the six-line intro, the `IF_TRY_TO_TAKE_THE_GLUE` interrupt branch, the slashes-on-body keyword injection through `dialog.rsp[0x194]` / `0x195`, the `HAVE_BOTH_LEDGERS` / casket / blackmail response menus, the room-specific ledger portrait override, and the ordered late-game one-shot bark chain through Karin, Butcher, Jimmy, and the day-5-only typed-topic branches.
  - The remaining work is the consistency audit across the other `handle_talk_to_*` ports to confirm which handlers still diverge materially from the corrected native control flow.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The next audit pass should start with the remaining keyword-loop handlers, especially `handle_talk_to_hank` and `handle_talk_to_stephanie`, to confirm whether their existing engine ports already cover the corrected native menu transitions and late-game one-shot chains.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
