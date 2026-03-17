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

- Recovered Mom's hidden keyword-buffer loop directly from raw Ghidra disassembly and aligned the startup runtime with the confirmed native menu state machine.
  - Confirmed `handle_talk_to_mom @ 0x31140` really does seed `g_dialogue_keyword_list_spec` from zero-based `dialog.rsp` line `0x102` in code, not just from the observed DOSBox kitchen screenshot; the hidden pre-menu seeds are now bounded to `0x102` (intro), `0x104` (same-day follow-up), `0x105` (later default), `0x113` (day 5), and `0x114` (day 6).
  - Confirmed the earlier Ghidra decompiler view understated Mom's topic-state machine: hidden inline copy/jump blocks loop back to `run_dialogue_keyword_menu` and rewrite the keyword buffer through `0x116->0x118`, `0x119->0x11a`, `0x11d->0x120`, `0x121->0x122`, `0x123->0x124`, `0x125..0x128->0x12a`, `0x12b->0x12c`, `0x12d->0x12e`, `0x12f->0x130`, `0x131..0x133->0x134`, `0x135->0x136`, `0x137->0x13b`, `0x13c->0x13e`, `0x13f->0x140`, `0x141->0x144`, `0x145..0x147->0x148`, `0x14b..0x14c->0x150`, `0x151..0x152->0x154`, and `0x15b->0x15c`.
  - ScummVM now follows those confirmed Mom menu seeds/transitions instead of unconditionally seeding `0x102` and falling through to `0x26c6` after every recognized topic.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the Mom menu-loop update.

## Next Suggested Action

1. Restore the remaining hidden Mom multi-line exchanges that sit next to the confirmed keyword-buffer rewrites.
   - Raw disassembly shows the menu/state transitions clearly, but several Mom branches still have extra `play_dialogue_line` calls in those hidden blocks that the current engine only partially mirrors.
2. Audit the remaining startup NPC talk handlers for the same direct-file `dialog.rsp` / `dialogue.idx` / `VOICE/*.CMP` split and hidden keyword-loop blocks.
   - Hank and Mom now show the pattern explicitly; the other keyword-driven handlers still need the same parity pass before broader dialogue cleanup.
