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

- Rebuilt Hank's keyword loop from the corrected native flow, labeled the recovered topic tails at `0x34490`, `0x34775`, and `0x34b1b`, and replaced the old flat topic table with the native reseed/menu state machine.
  - Hank now preserves the hidden `STEVE -> 0xd7`, `LOUSY RAT -> 0xda`, `'LIGHTING OUT' -> 0xdd -> 0xde/0xdf`, `DADDY -> 0xe2`, `GIRL -> 0xe4`, `TV -> RANGSHOT -> 0xe8`, `COWBOY SHOW -> 0xea`, `SICK -> 0xec`, `0xee/0xef/0xf0 -> 0x8a0..0x8b3`, and `lodge -> 0x8bf..0x8d3` loop paths instead of collapsing every handled topic through the generic `0xa32 -> 0x8dc` exit.
  - The next corrective pass is the remaining keyword-loop / event-tail audit, starting with `handle_talk_to_mom`.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_mom`, then the rest of the remaining `handle_talk_to_*` ports whose keyword loops or event tails may still be truncated by the old `play_dialogue_line` decompile breakage.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
