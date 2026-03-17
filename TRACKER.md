# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `910`
- Named/documented: `592`
- Still `FUN_*` / undocumented: `318`

## Last Confirmed Action

- Rebuilt `handle_talk_to_phelps @ 0x29920` from the corrected native flow and updated the native notes for its restored response menus and one-shot chain.
  - Phelps now preserves the six-line intro, the pre-day-5 response menu at `dialog.rsp[0x22a]`, the multi-line Whaley / casket / quarter item branches, the quarter-side action tags `GET_PRN_MAG` and `DEL_DRT_MAG`, and the ordered late-game one-shot bark chain through the Jimmy, Butcher, Moynahan, and Karin outcomes.
  - The next highest-value corrective pass is `handle_talk_to_moynahan`, whose native function still hides a much larger keyword/menu block and several item-driven response menus than the current engine port reflects.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_moynahan`, then the other remaining handlers that still hide native menu/event logic behind mistyped `play_dialogue_line` calls or orphan `run_dialogue_response_menu` / `run_dialogue_keyword_menu` blocks.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
