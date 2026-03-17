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

- Audited the native `handle_talk_to_hank` and `handle_talk_to_jimmy` item branches against the current engine code and fixed the confirmed shared-evidence state mismatch.
  - Native `handle_talk_to_hank @ 0x33a30` sets the shared casket-photo and Whaley/Herrill evidence wrappers before its `0xa3e` / `0xa53` lines; the engine now mirrors that by setting the matching `SHOWN_PHOTO_OF_CORPSE_AROUN` and `SHOWN_PHOTO_OF_WHALEY_HERRI` runtime flags instead of dead Hank-local placeholder bools.
  - Native `handle_talk_to_jimmy @ 0x28880` sets the shared ledger, casket-photo, note/checkbook, and Whaley/Herrill evidence wrappers on the corresponding item branches; the engine now mirrors those writes through the matching `SHOWN_*` runtime flags before `0x4b00`, `0x4b21`, and `0x4af8`.
  - Confirmed the tempting Jimmy intro tweak was wrong: the native handler does **not** set the newspaper handoff state on the first no-item `0x4a4c` path, so that engine-side gate was intentionally left unchanged.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Hank/Jimmy parity update.

## Next Suggested Action

1. Carry the same shared-evidence/runtime-flag audit into the remaining item-heavy talk handlers.
   - Native notes already tie Dwayne, Edna, Herrill, Johnson, Mom, and related handlers to the same note/checkbook, ledger, casket-photo, and Whaley/Herrill evidence wrappers.
   - The engine-side branches for those NPCs should be checked for missing `SHOWN_*` runtime flag writes before more dialogue parity work builds on them.
2. Resume the unresolved raw-byte audit on Mom's helper-heavy hidden branches.
   - `0x11d` and `0x121` still include hidden helper calls (`0x388d0`, `0x388c0`, `0x38420`) and the `BABY_GURGLE` speaker path that the engine still only mirrors partially.
