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

- Audited `handle_talk_to_mom` against the corrected native control flow and confirmed the current rebuilt port already covers the recovered intro, Jimmy-absent, day-5, and keyword-tail continuations without needing another engine-side correction.
- Corrected `handle_talk_to_stephanie` so the hidden zero-based `dialog.rsp[0x2f9..0x2fd]` spyhole continuation only takes over once the live keyword buffer has advanced to `dialog.rsp[0x2f8]`; the earlier zero-based `dialog.rsp[0x2e4]` `BYE` remains the direct `0x46a4` exit in the pre-spyhole states.
  - The matching native annotation on `handle_talk_to_stephanie @ 0x36710` now needs to record that `0x2e4` and `0x2fd` are distinct BYE branches selected by control-flow stage, not a single generic BYE string check.
  - The next corrective pass is the hidden `handle_talk_to_wasp_woman` tail, where the raw native range now confirms the current engine port missed the `0x4c00` intro continuation, the initial zero-based keyword seed `dialog.rsp[0x300]`, and the later `0x304` / `0x309` / `0x30b` topic-buffer rewrites.

## Next Suggested Action

1. Rebuild `handle_talk_to_wasp_woman` from the newly recovered hidden native tail.
   - The raw instruction range at `0x2ff20..0x3030b` confirms the intro-only `0x4c00` continuation, initial zero-based keyword seed `dialog.rsp[0x300]`, later keyword-buffer rewrites to `0x304`, `0x309`, and `0x30b`, the hidden `C027A.FST` playback after the `0x302` topic, and the explicit late `0x30e` farewell line `0x4c9a`.
2. Revisit `handle_talk_to_moynahan` after the Wasp rebuild and decide whether the remaining `0x199` / `0x1c4` BYE ambiguity needs another native-stage correction or a runtime-side selection-context helper.
3. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
