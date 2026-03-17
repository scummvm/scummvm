# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `887`
- Named/documented: `565`
- Still `FUN_*` / undocumented: `322`

## Last Confirmed Action

- Completed the shared-evidence/runtime-flag audit for the currently implemented startup-room item handlers and fixed the remaining Mom mismatch.
  - Native `handle_talk_to_mom @ 0x31140` sets the shared note/checkbook, ledger, casket-photo, Whaley/Herrill, and TV-deed evidence wrappers before `0x2317`, `0x2320`, `0x233b`, `0x239c`, and `0x2382`; the engine now mirrors those writes through `SHOWN_EVIDENCE_OF_BLACKMAIL`, `SHOWN_LEDGERS_TO_ANYONE_OTH`, `SHOWN_PHOTO_OF_CORPSE_AROUN`, `SHOWN_PHOTO_OF_WHALEY_HERRI`, and `SHOWN_EVIDENCE_SHERIFF_OWNS`.
  - The earlier Hank/Jimmy parity pass remains correct: Hank now mirrors the shared casket-photo / Whaley-Herrill evidence writes, Jimmy now mirrors the shared ledger / casket-photo / note-checkbook / Whaley-Herrill evidence writes, and Jimmy's first no-item `0x4a4c` path still intentionally leaves the newspaper handoff state clear because the native handler does too.
  - Native audits of Dwayne, Edna, Herrill, and Johnson confirm the same shared wrapper pattern, but those NPC-specific startup handlers are not yet implemented in `startup_dialogue.cpp`, so there was no additional engine-side evidence write to patch in this pass.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Mom evidence-state update.

## Next Suggested Action

1. Resume the unresolved raw-byte audit on Mom's helper-heavy hidden branches.
   - `0x11d` and `0x121` still include hidden helper calls (`0x388d0`, `0x388c0`, `0x38420`) and the `BABY_GURGLE` speaker path that the engine still only mirrors partially.
2. If wider evidence parity work is needed beyond Mom/Hank/Jimmy, port the next native talk handler cluster into `startup_dialogue.cpp` before auditing its shared `SHOWN_*` writes.
   - Dwayne, Edna, Herrill, and Johnson already have confirmed native evidence-wrapper behavior, but there is no explicit engine-side room handler for them yet.
