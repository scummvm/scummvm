# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `908`
- Named/documented: `590`
- Still `FUN_*` / undocumented: `318`

## Last Confirmed Action

- Rebuilt `handle_talk_to_boyle @ 0x2cb20` from the corrected native flow and synchronized the recovered shared state wrapper rename `get_set_shared_dialogue_state_d2ebc @ 0x382d0`.
  - Boyle's engine port now covers the native intro response-menu chain, keyword-buffer seeding, one-shot no-item followups, the hidden `Arsonist` alias in the keyword loop, and the full `BOYLES_BUTTON` / blackmail-evidence / `GASCAN` / photo item logic.
  - The next highest-value corrective pass is the adjacent Pottsdam branch chain in `handle_talk_to_mrs_potts`, which is already tied to the recovered Tuesday-night alibi wrapper.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_mrs_potts`, then the other remaining handlers that still hide native menu/event logic behind mistyped `play_dialogue_line` calls or orphan `run_dialogue_response_menu` / `run_dialogue_keyword_menu` blocks.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
