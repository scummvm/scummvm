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

- Rebuilt `handle_talk_to_mrs_potts @ 0x2ee70` from the corrected native flow and turned the orphan helpers at `0x382c0` and `0x38920` into named functions: `get_set_shared_dialogue_state_d2eb8` and `play_c040_fst`.
  - Mrs. Potts now preserves the native intro response menu, the room-specific dead-Stephanie branches, the `REMAINS` death sequence through `C040.FST` and `DIE_IN_CHAIR`, the `KARIN_KIDNAPED` response menu, and the hidden keyword loop over `dialog.rsp[0x237..0x24a]`.
  - The next highest-value corrective pass is `handle_talk_to_phelps`, whose native handler still contains response menus, multi-line evidence branches, and a large no-item one-shot chain that the current engine port flattens away.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_phelps`, then the other remaining handlers that still hide native menu/event logic behind mistyped `play_dialogue_line` calls or orphan `run_dialogue_response_menu` / `run_dialogue_keyword_menu` blocks.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
