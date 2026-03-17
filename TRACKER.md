# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `890`
- Named/documented: `566`
- Still `FUN_*` / undocumented: `324`

## Last Confirmed Action

- Completed the follow-up audit on the remaining Mom/Wasp Woman helper cluster after the Mom `0x11d` / `0x121` recovery.
  - `get_set_wasp_woman_dialogue_state_d2f10 @ 0x38420` is no longer treated as a Mom/Wasp shared helper: a linear call scan only finds `CALL 0x38420` at `0x2ff4e` and `0x3010f`, both inside hidden `handle_talk_to_wasp_woman @ 0x2fde0`, so the Ghidra name was corrected accordingly.
  - Those two Wasp Woman call sites gate the no-item intro exchange (`0x4bee`, optional `0x4bf2`, then `0x4bf6` / `0x4bfc`) and the hidden `0x305` keyword branch (`0x4c31` followed by the `0x307` response-menu lines `0x4c4d` / `0x4c53` or `0x4c5e`).
  - `play_c008_prelude_fst @ 0x388c0` is confirmed as a `GRAPHIC/FST/C123.FST` wrapper, but both normal xrefs and a whole-program call scan still show no direct callers in the current database.
  - This pass was Ghidra/documentation-only; no engine build was needed after the audit.

## Next Suggested Action

1. Port `handle_talk_to_wasp_woman @ 0x2fde0` into `startup_dialogue.cpp`.
   - The engine only has explicit room dialogue handlers for Jimmy, Mom, and Hank today; Wasp Woman is now the best-bounded next target, including the `0x38420`-gated intro path and the hidden `0x305` -> `0x307` response-menu branch.
2. After Wasp Woman, port the next bounded startup-room handler cluster rather than jumping back to broad helper scans.
   - `handle_talk_to_pta_mom @ 0x34e30` is already identified, while Dwayne, Edna, Herrill, and Johnson still need explicit engine-side room handlers before their shared `SHOWN_*` evidence writes can be mirrored.
