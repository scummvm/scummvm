# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `906`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `319`

## Last Confirmed Action

- Rebuilt `handle_talk_to_whaley @ 0x23ec0` from corrected native control flow and fixed the engine port to follow the recovered event, menu, and keyword branches.
  - Whaley now covers the busted-screwing event chain, the full evidence-item exchanges, the intro/revisit keyword setup, the Steph/day-5/Karin/BURNED_TV_STATION one-shots, and the looping `dialog.rsp` topic transitions.
  - The engine-side shared dialogue state now carries the Dwayne/Whaley discipline follow-up bit, and the matching native FST helpers (`C052`, `C053`, `C055`, `C057`) are named in Ghidra.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value follow-ups are the remaining short ports that still hide native menu/event logic behind mistyped `play_dialogue_line` calls, starting with `handle_talk_to_mr_potts`, `handle_talk_to_jimmy`, and the other handlers that still xref `run_dialogue_response_menu` / `run_dialogue_keyword_menu` from not-yet-audited code ranges.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
