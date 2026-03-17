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

- Fixed the dialogue crash introduced by the recovered movie-surface switch.
  - The native `play_fst_sequence @ 0x72550` surface swap was correct, but `DialogueSystem::runRoomNpcDialogue` had cached `_engine.getScreen()` before FST playback and kept using that freed `Graphics::Screen` after `setDisplayMode()` recreated it.
  - Dialogue rendering now resolves the active screen on demand for palette, blit, and cursor draws after FST playback returns, so native-style dialogue movie playback can switch `320x200` movie mode in and back out without leaving stale screen pointers behind.

## Next Suggested Action

1. Exercise the remaining in-game FST callers against the shared movie-surface switch and confirm whether any of them rely on an additional redraw step after playback returns to the `640`-wide gameplay UI.
   - The wrapper and dialogue system now both survive the mode transition, but the surrounding caller-side redraw expectations still need a quick pass across other dialogue-triggered clips and death-FST playback.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of that next confirmed thread before the next commit.
