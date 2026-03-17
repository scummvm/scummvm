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

- Recovered the hidden Mom multi-line/menu branches directly from raw byte disassembly and aligned the startup runtime with the confirmed native call order instead of the earlier truncated Ghidra listing.
  - Confirmed the previously recorded `0x137->0x13b` rewrite was wrong: `0x137` loops on the same keyword buffer after `0x1f7f` / `0x1f84` / `0x1f8a` / `0x1f8e`, while `0x13a` is the branch that really rewrites the buffer to `0x13b`.
  - Confirmed hidden multi-line Mom exchanges on `0x116`, `0x119`, `0x125..0x128`, `0x12b`, `0x12d`, `0x12f`, `0x131..0x133`, `0x135`, `0x137`, `0x145..0x147`, `0x14a`, `0x155`, and `0x156`; those branches are now mirrored in ScummVM with the directly observed speaker strings and portrait variants.
  - Confirmed `0x141` is a Lodge-topic response-menu branch: it calls the shared `get_set_discussed_lodge_topic(0, 1)` wrapper at `0x384c0`, plays `0x1fe1` / `0x1fe5` / `0x1feb` / `0x1fef`, shows zero-based `dialog.rsp` line `0x142`, then rewrites to `0x144`; the `STEPH_MIDGAME_PLAYED` path instead plays `0x2154` / `0x215a` and loops without rewriting.
  - Confirmed `0x151..0x152` is another hidden response-menu branch: it plays `0x20f7` / `0x20fb`, shows zero-based `dialog.rsp` line `0x153`, keeps the same keyword buffer after response `1`, and only rewrites to `0x154` after response `2`.
  - The engine-side Lodge-topic state assumption is now cleaned up to match native scope: the prior dead Hank-local flag was replaced with a shared runtime flag, and the day-5 `GOOD CAUSE` sequence now uses the directly confirmed Mom portrait variant for `0x21a8`.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Mom hidden-branch update.

## Next Suggested Action

1. Resolve the still-partial helper-heavy Mom branches before widening the audit.
   - `0x11d` and `0x121` still include hidden helper calls (`0x388d0`, `0x388c0`, `0x38420`) and the `BABY_GURGLE` speaker path that the engine still only mirrors partially.
   - `get_set_mom_good_cause_day5_state @ 0x383c0` is confirmed as a real wrapper over a native global, but the concrete setter path is still outside the recovered call graph; the engine still uses a local placeholder bool there.
2. Audit the remaining startup NPC talk handlers for the same direct-file `dialog.rsp` / `dialogue.idx` / `VOICE/*.CMP` split and hidden keyword-loop blocks.
   - Hank and Mom now expose the pattern clearly; Dwayne, Potts, Stephanie, and the other keyword-driven handlers still need the same parity pass before broader dialogue cleanup.
