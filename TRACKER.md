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

- Rebuilt `handle_talk_to_johnson @ 0x27dd0` from the corrected native flow, labeled the recovered reseed tails at `0x28699`, `0x28708`, and `0x28760`, and fixed the engine-side keyword loop so the hidden topic-buffer rewrites are preserved instead of collapsing straight to the generic exit.
  - Johnson now keeps `0x180` / `0x181` / `0x182` on the `0xb3a -> dialog.rsp[0x183]` path, `0x184` / `0x185` on the `0xb46 -> dialog.rsp[0x186]` path, and `0x187` on the `PC 0xb52` / `JOHNSON 0xb56` / `dialog.rsp[0x188]` path; only `0x177` is the direct exit, while `0x189` remains a silent loop.
  - The next corrective pass is the remaining keyword-loop / event-tail audit, starting with `handle_talk_to_herrill`.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_herrill`, then the rest of the remaining `handle_talk_to_*` ports whose keyword loops or event tails may still be truncated by the old `play_dialogue_line` decompile breakage.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
