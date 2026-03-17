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

- Rebuilt `handle_talk_to_stephanie @ 0x36710` from the corrected native flow, labeled the recovered orphan tails at `0x37650` and `0x37e78`, and wired Dwayne's completed Karin-alive follow-up into shared dialogue state so Stephanie can see it.
  - Stephanie now preserves the day-gated intro ladder, the hidden TV-station / SCRATCHED_TUCKER / BOLT_OF_CLOTH / BARBER_POLE / DINER / jail response menus, the `0x2f9..0x2fd` spyhole keyword continuation through `0x4a19` / `0x4a1e` / `0x4a23`, the post-`KARIN_KIDNAPED` Potts-alibi follow-up block, and the extended Karin outcome tail through `0x49c1` / `0x49ea` / `0x49ef`.
  - The next corrective pass is now the audit of handlers that feed Stephanie's recovered shared follow-up states, starting with `handle_talk_to_mrs_potts`, because the engine still has a `discussedMrsPottsTuesdayNightAlibi` state that is never written.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value immediate follow-up is `handle_talk_to_mrs_potts`, then any other previously rebuilt handlers that still leave recovered shared dialogue states unwired in the engine-side ports.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
