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
- Rebuilt `handle_talk_to_wasp_woman` from the recovered hidden tail at `0x2ff20..0x3030b`.
  - The engine port now restores the intro-only `0x4c00` continuation, keeps Wasp Woman's keyword-buffer stage persistent across revisits, and applies the native `0x300` -> `0x304` / `0x309` / `0x30b` buffer rewrites plus the hidden `C027A.FST` playback after the `0x302` topic.
  - The raw native annotation on `handle_talk_to_wasp_woman @ 0x2fde0` now needs to record the real late-loop behavior: the initial `dialog.rsp[0x301]` `BYE` exits silently, the later `dialog.rsp[0x30e]` `BYE` loops silently, and only the fallback path plays `0x4c9a`.
- Audited `handle_talk_to_moynahan` against the fully disassembled raw loop at `0x2563c..0x25f1d`.
  - The hidden zero-based `dialog.rsp[0x1c4]` compare at `0x25ef3` is real, but the shipped resource gives both `0x199` and `0x1c4` the same literal `BYE` bytes. The `0x1c4` match only jumps back into the top-of-loop `0x199` test, so it does not create another distinct engine-visible branch beyond the already ported `0x3e32` farewell.
  - That closes the outstanding `handle_talk_to_*` corrective-pass audit for the current dialogue subsystem sweep.

## Next Suggested Action

1. Pick the next confirmed dialogue-analysis thread outside the completed `handle_talk_to_*` corrective-pass sweep.
   - The talk-handler audit is now closed for the currently suspicious ports (`mom`, `stephanie`, `wasp_woman`, and `moynahan`), so the next useful step should come from another confirmed native gap rather than another speculative BYE hunt.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of that next confirmed thread before the next commit.
