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

- Recovered the native dialogue resource split and the Mom/Hank keyword-buffer state transitions, then aligned the startup runtime with the confirmed Mom menu path.
  - Confirmed `load_dialogue_response_line @ 0x3a1a0` reads keyword/menu text from direct file `dialog.rsp`, while `load_dialogue_index @ 0x79a20` and `play_dialogue_line @ 0x7a690` map wav ids through direct `dialogue.idx` subtitle records plus `VOICE/*.CMP` audio, not DAT-backed archive paths.
  - Confirmed `handle_talk_to_hank @ 0x33a30` seeds `g_dialogue_keyword_list_spec` from `dialog.rsp` lines `0xc8`, `0xc9`, or `0xca` after the opening response menu and rewrites it to line `0xd5` after the hidden `0x725` / `0x729` / `0x72d` / `0x733` / `0x737` / `0x73b` / `0x741` / `0x747` branch.
  - Confirmed `handle_talk_to_mom @ 0x31140` compares keyword selections against `dialog.rsp` lines `0x116` through `0x171`, rewrites the keyword buffer to line `0x13e` after the `0x1faf` / `0x1fb4` / `0x1fb9` / `0x1fbd` Slaughterhouse exchange, and rewrites it to line `0x140` after the `FATHER` branch; the observed early-game DOSBox kitchen menu matches zero-based `dialog.rsp` line `0x102` exactly.
  - ScummVM now seeds Mom's keyword menu from that confirmed `dialog.rsp` line, restores the hidden Slaughterhouse follow-up sequence and menu rewrite, and updates the confirmed Mom portrait-variant calls on the intro, Cookies, and Sparky exchanges.
  - `rtk make -C /Users/alex/Workspace/scummvm/build-vscode-harvester-debug -j4` succeeded after the dialogue/resource alignment work.

## Next Suggested Action

1. Isolate Mom's initial `g_dialogue_keyword_list_spec` seed in Ghidra instead of relying on the observed DOSBox `dialog.rsp` line match.
   - Static disassembly already confirms the later `0x13e` and `0x140` rewrites plus all keyword comparisons, but the first native buffer write that produces the observed day-1 line `0x102` has not been recovered cleanly yet.
2. Audit the remaining startup NPC talk handlers for the same direct-file `dialog.rsp` / `dialogue.idx` / `VOICE/*.CMP` split.
   - Hank is now bounded (`0xc8` / `0xc9` / `0xca` -> `0xd5`) and Mom is partially bounded, but the other keyword-driven handlers still need the same parity pass before broader dialogue cleanup.
