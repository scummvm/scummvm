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

- Rebuilt `handle_talk_to_mrs_potts @ 0x2ee70` from the corrected native flow, labeled the recovered Karin-alibi and grounded-keyword tails at `0x2f694` and `0x2f9d1`, and wired Mrs. Potts's Tuesday-night alibi follow-up into shared dialogue state so Stephanie can see it.
  - Mrs. Potts now preserves the native intro default, the non-dead one-shot event gates through the keyword loop, the hidden `Pearls` / `Bake Sale` / `Wedding` / `Grounded` topic expansions and reseeds, the later `Mr. Pottsdam` / `Moynahan` / `Lodge` reseeds, and the `KARIN_KIDNAPED` response-1 alibi setter through `0x28f2`.
  - The next corrective pass is now the remaining talk-handler audit beyond the Potts family/Stephanie shared-state chain, starting with `handle_talk_to_johnson`.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_johnson`, then the rest of the remaining `handle_talk_to_*` ports whose keyword loops or event tails may still be truncated by the old `play_dialogue_line` decompile breakage.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
