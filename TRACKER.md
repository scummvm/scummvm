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

- Rebuilt `handle_talk_to_mr_potts @ 0x26000` from corrected native control flow and fixed the engine port to match the recovered event, response-menu, and keyword-loop branches.
  - Mr. Potts now covers the POTTSDAM digging branches, evidence-item exchanges, auxiliary intro and revisit flow, the Karin/day-event one-shots, the persistent keyword buffer, the runtime-built accusation menu, and the hidden peephole / murder / molest topic logic.
  - The engine dialogue runtime now supports native-style response-menu parsing for dynamically assembled menu text, and Ghidra now names the `C016A.FST` helper (`play_c016a_fst`) with updated notes on the broken Mr. Potts function stub.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_jimmy`, then the other remaining short handlers that still hide native menu/event logic behind mistyped `play_dialogue_line` calls or orphan `run_dialogue_response_menu` / `run_dialogue_keyword_menu` blocks.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
