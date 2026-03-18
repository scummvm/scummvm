# TRACKER

## Current Focus

- Verify NPC-specific dialogue handlers against their native hardcoded dispatchers before assuming they should use the generic keyword flow
- Keep the engine-side Jimmy conversation aligned with the native short-circuit branch order instead of letting special barks fall through
- Re-check only the remaining in-game drag/drop and dialogue edge cases that still need native capture confirmation

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited the native Jimmy conversation path in Ghidra and confirmed it is a dedicated hardcoded handler, not a generic keyword-loop dialogue.
  - `handle_talk_to_jimmy` is a short-circuit chain: first no-item talk returns `0x4a4c` or `0x4a58`, special no-item cases for `SNEAKERS` and `PAPER_CHK_4/3/2` each terminate immediately on their own line, and only the final fallback returns `0x4b38`.
  - The original Jimmy handler does not call the keyword or response menu helpers directly, so there is no evidence that bare click-talk should fan out into a larger dynamic conversation tree.
- Adjusted the engine-side Jimmy handler to match that native branch behavior more closely.
  - Jimmy's special no-item barks now return immediately instead of falling through to later lines, preserving the native priority order.

## Next Suggested Action

1. Compare the native and engine Jimmy talk paths in-game with and without `SNEAKERS`, `BROOMKEY`, and `PAPER_CHK_2/3/4` set to confirm the branch priorities now match the DOS behavior.
2. If another NPC still looks too shallow or too chatty versus DOSBox, verify that NPC's dedicated handler in Ghidra before changing any engine dialogue flow assumptions.
