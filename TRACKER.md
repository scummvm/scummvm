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

- Recovered and ported Mom's hidden `0x11d` / `0x121` branches instead of the old one-line stubs.
  - `0x11d` now mirrors the native `0x1e7c` / `0x1e80` exchange, `C008A.FST`, `0x1e8e` / `0x1e94`, the `BABY_GURGLE` action-tag dispatch, `0x1e9d` / `0x1ea3`, `C008B.FST`, and the keyword-buffer rewrite to `0x120`.
  - `0x121` now mirrors the native `0x1ec0` / `0x1ec4` / `0x1ec9` / `0x1ecd` exchange before rewriting the keyword buffer to `0x122`.
  - The raw-byte audit corrected the earlier helper guess: `0x388d0` and `0x388e0` belong to Mom `0x11d`, while `0x388c0` and `0x38420` are real nearby helpers but not part of Mom `0x11d` / `0x121`.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Mom hidden-branch recovery.

## Next Suggested Action

1. Audit the remaining hidden Mom/Wasp Woman helper cluster that is still unresolved after closing Mom `0x11d` / `0x121`.
   - `play_c008_prelude_fst @ 0x388c0` and `get_set_wasp_woman_mom_dialogue_state_d2f10 @ 0x38420` are confirmed helpers, but the raw-byte recovery showed they are not part of Mom `0x11d` / `0x121`; identify which hidden Mom/Wasp Woman topics actually call them.
2. If wider evidence parity work is needed beyond Mom/Hank/Jimmy, port the next native talk handler cluster into `startup_dialogue.cpp` before auditing its shared `SHOWN_*` writes.
   - Dwayne, Edna, Herrill, and Johnson already have confirmed native evidence-wrapper behavior, but there is no explicit engine-side room handler for them yet.
