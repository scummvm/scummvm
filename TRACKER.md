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

- Corrected `handle_talk_to_sparky` so only zero-based `dialog.rsp[0x2c3]` is the direct `0x39e` exit; zero-based `dialog.rsp[0x2ce]` once again follows the native silent-loop branch instead of being swallowed by a generic BYE string check.
  - The matching native annotation on `handle_talk_to_sparky @ 0x39240` now records that `0x2c3` and `0x2ce` are distinct BYE branches and must not be collapsed together.
  - The next corrective pass is the remaining audit of handlers that still looked suspicious after the larger rebuilds, starting with `handle_talk_to_mom`.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_mom`, then the remaining small-loop handlers (`wasp_woman`, `moynahan`, `stephanie`) to confirm whether any non-committed corrective tails are still left after the current rebuild set.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
