# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `908`
- Named/documented: `589`
- Still `FUN_*` / undocumented: `319`

## Last Confirmed Action

- Re-audited `handle_talk_to_jimmy @ 0x28880` against the corrected native flow and fixed the short-circuit branches that the earlier engine port had collapsed together.
  - Jimmy's first no-item visit, first `NEWSPAPER` handoff, and `SNEAKERS` item branch now return on the native single-line outcomes instead of falling through into later bark logic.
  - The native notes now record Jimmy's compact branch layout, and the next highest-value corrective pass is the still-incomplete Boyle handler.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_boyle`, then the other remaining short handlers that still hide native menu/event logic behind mistyped `play_dialogue_line` calls or orphan `run_dialogue_response_menu` / `run_dialogue_keyword_menu` blocks.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
