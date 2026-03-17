# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `899`
- Named/documented: `586`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Corrected `handle_talk_to_herrill` so only zero-based `dialog.rsp[0x283]` is the direct BYE exit; zero-based `dialog.rsp[0x293]` once again follows the native `0x3099 -> 0x2ff4` path instead of being swallowed by a generic BYE string check.
  - The matching native annotation on `handle_talk_to_herrill @ 0x2dc00` now records that `0x283` and `0x293` are distinct BYE branches and must not be collapsed together.
  - The next corrective pass is the remaining keyword-loop / event-tail audit, starting with `handle_talk_to_hank`.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_hank`, then the rest of the remaining `handle_talk_to_*` ports whose keyword loops or event tails may still be truncated by the old `play_dialogue_line` decompile breakage.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
