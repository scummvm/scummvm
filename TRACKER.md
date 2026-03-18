# TRACKER

## Current Focus

- Verify dialogue-resource selection against native expectations before changing NPC branch logic
- Keep Hank's keyword and response flow aligned with the native `DIALOG.RSP` table the executable actually expects
- Re-check only the remaining in-game drag/drop and dialogue edge cases that still need native capture confirmation

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited Hank's talk path in Ghidra and confirmed the native issue is data selection, not a missing generic conversation tree.
  - `handle_talk_to_hank` is a native keyword/response-menu conversation that expects a `DIALOG.RSP` where zero-based line `0xd2` is `Mom`, `0xd6` is `STEVE`, `0xeb` is `SICK`, and `0xf2` is `bye`.
  - The duplicate `DIALOG.RSP` under `iso/Harvester` does not match that layout, while the root-game `DIALOG.RSP` does, which explains why a visible `Mom` selection can fall into Hank's generic `0xa32` plus `0x8dc` fallback when the wrong copy is loaded.
- Adjusted startup dialogue-text loading to prefer the native-compatible ancestor/root `DIALOG.RSP` when the initially resolved table does not match Hank's verified native signature.

## Next Suggested Action

1. Re-test Hank's `Mom`, `TV`, and `Sick` topic paths in-game and confirm the keyword menu now reaches the native follow-up trees instead of the generic fallback pair.
2. If another NPC still diverges, compare the active `DIALOG.RSP` lines for that branch before changing the handler logic itself.
